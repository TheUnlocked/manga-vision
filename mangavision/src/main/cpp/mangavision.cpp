//
// Created by Unlocked on 8/7/2025.
//

#include <android/bitmap.h>
#include <opencv2/imgproc.hpp>

#include "mangavision.h"

extern "C" JNIEXPORT jboolean JNICALL
Java_com_github_theunlocked_mangavision_SpreadDetector_nIsSpread(
        JNIEnv* env,
        jclass,
        jobject left,
        jobject right
) {
    cv::Mat mat1, mat2;
    bitmapToMat(env, left, mat1);
    bitmapToMat(env, right, mat2);

    preprocess(mat1);
    preprocess(mat2);

    // There are two main heuristics we'll use to determine if two adjacent pages comprise a spread:
    // 1. "Interestingness"
    //      A group of pixels is considered "interesting" when it has non-zero variance.
    //      Interestingness is tested in sets of pixels within a column close to the center edge.
    //      Note that quantization during pre-processing helped ensure that compression artifacts in
    //      the image wouldn't cause entirely non-interesting parts of the image to have non-zero variance.
    // 2. "Seam discontinuity"
    //      The purpose of this heuristic is to differentiate between a continuous image across the seam versus
    //      unrelated lines or textures that simply happen to run to the center of the page.
    //      To detect that, the two sides are fused, and then an edge map is created using Canny edge detection.
    //      When there's a discontinuity between the left and right sides of the seam, we expect the center of the
    //      edge map to contain continuous vertical lines. If, on the other hand, the center of the edge map is more
    //      noisy and discontinuous, those edges are likely part of texture in the image unrelated to the seam.
    //      A kernel is convolved on the center columns of the edge map to intensify vertical structures, then the
    //      result is subtracted from the original edge map. For highly vertical lines, this will be very similar to
    //      the original so the resulting image will be dark, whereas for more noisy edge maps, more pixels will
    //      be unaffected and so it will be brighter. The ratio of lit pixels between the original edge map and
    //      the difference is the value of this heuristic (a measure of how "discontinuous" the vertical seam is).

    // An image is predicted to be a spread if:
    //      1. Both sides have a number of interesting sub-regions above a certain threshold
    //      2. The regions of each side that are interesting line up, within a tolerance
    //      3. The seam discontinuity is above a certain threshold (i.e. there is not a clear seam)

    auto leftInteresting = getInterestingnessMap(mat1.col(mat1.cols - 1));
    if (std::count(leftInteresting.begin(), leftInteresting.end(), true) < MIN_INTERESTING_REGION_COUNT) {
        return false;
    }

    auto rightInteresting = getInterestingnessMap(mat2.col(0));
    if (std::count(rightInteresting.begin(), rightInteresting.end(), true) < MIN_INTERESTING_REGION_COUNT) {
        return false;
    }

    int interestingnessDifference = 0;
    for (int i = 0; i < leftInteresting.size(); i++) {
        if (leftInteresting[i] ^ rightInteresting[i]) {
            interestingnessDifference++;
        }
    }
    if (interestingnessDifference > MAX_DIFFERING_REGION_COUNT) {
        return false;
    }

    if (getSeamDiscontinuity(mat1, mat2) < SEAM_DISCONTINUITY_THRESHOLD) {
        return false;
    }

    return true;
}

std::array<bool, INTERESTINGNESS_REGION_COUNT> getInterestingnessMap(const cv::Mat& mat) {
    int regionSize = mat.rows / INTERESTINGNESS_REGION_COUNT;
    std::array<bool, INTERESTINGNESS_REGION_COUNT> map {};

    for (int i = 0; i < INTERESTINGNESS_REGION_COUNT; i++) {
        cv::Scalar stddev;
        cv::meanStdDev(
                mat.rowRange(regionSize * i, regionSize * (i + 1)),
                cv::Scalar(), stddev);
        map[i] = stddev[0] != 0;
    }

    return map;
}

double getSeamDiscontinuity(const cv::Mat& mat1, const cv::Mat& mat2) {
    constexpr int centerBandSize = 16;
    constexpr int halfBandSize = centerBandSize / 2;

    cv::Mat fused(mat1.rows, centerBandSize, mat1.type());
    cv::hconcat(
            mat1.colRange(mat1.cols - halfBandSize, mat1.cols),
            mat2.colRange(0, halfBandSize),
            fused);

    cv::Mat edges(fused.size(), CV_8UC1);
    cv::Canny(fused, edges, CANNY_THRESHOLD_MIN, CANNY_THRESHOLD_MAX);

    // Keeping this as a "matrix header" makes filter2D work incorrectly, so we need to clone it.
    cv::Mat centerEdges = edges.colRange(halfBandSize - 1, halfBandSize + 1).clone();

    double edgesMean = cv::mean(centerEdges).val[0];

    // Shouldn't happen in practice, but there is a hypothetical divide-by-zero if this case is unchecked
    if (edgesMean == 0) {
        return 0;
    }

    cv::Mat kernel = (cv::Mat_<double>(3, 3, CV_64FC1) <<
        0, 0.5, 0,
        -1, 1, -1,
        0, 0.5, 0
    );

    cv::Mat convolved(centerEdges.size(), centerEdges.type());
    cv::filter2D(centerEdges, convolved, -1, kernel);

    cv::subtract(centerEdges, convolved, convolved);
    double diffMean = cv::mean(convolved).val[0];

    return diffMean / edgesMean;
}

void resizeToProcessingResolution(cv::Mat &mat) {
    int newHeight = PROCESSING_RESOLUTION;
    if (mat.rows == newHeight) {
        return;
    }
    double scaleFactor = newHeight / (double)mat.rows;
    int newWidth = (int)round(mat.cols * scaleFactor);
    cv::resize(mat, mat, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA);
}

void preprocess(cv::Mat &mat) {
    resizeToProcessingResolution(mat);
    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::GaussianBlur(mat, mat, cv::Size(5, 5), 0);
    uniformQuantize(mat);
}

constexpr std::array<uchar, 256> generateQuantizationMap() {
    std::array<uchar, 256> quantizationMap {};
    int numBuckets = QUANTIZATION_LEVELS;
    int highestBucket = numBuckets - 1;
    for (int i = 0; i < 255; i++) {
        quantizationMap[i] = (i * numBuckets / 255) * 255 / highestBucket;
    }
    quantizationMap[255] = 255;
    return quantizationMap;
}

constexpr auto quantizationMap = generateQuantizationMap();

void uniformQuantize(cv::Mat &mat) {
    mat.forEach<uchar>([](uchar &p, const int*) -> void {
        p = quantizationMap[p];
    });
}

/**
 * Taken from https://github.com/opencv/opencv/blob/e31ff001042ea2321c3d67266a8d240dc836ecd2/modules/java/generator/src/cpp/utils.cpp#L26
 */
void bitmapToMat(JNIEnv* env, jobject bitmap, cv::Mat &dst) {
    try {
        AndroidBitmapInfo info;
        void* pixels = nullptr;

        CV_Assert( AndroidBitmap_getInfo(env, bitmap, &info) >= 0 );
        CV_Assert( info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                   info.format == ANDROID_BITMAP_FORMAT_RGB_565 );
        CV_Assert( AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0 );
        CV_Assert( pixels );

        dst.create((int)info.height, (int)info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            cv::Mat tmp((int)info.height, (int)info.width, CV_8UC4, pixels);
            if (info.flags & ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL) {
                tmp.copyTo(dst);
            }
            else {
                cvtColor(tmp, dst, cv::COLOR_mRGBA2RGBA);
            }
        }
        else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            cv::Mat tmp((int)info.height, (int)info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, cv::COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    }
    catch (const cv::Exception& e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
        return;
    }
    catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        env->ThrowNew(env->FindClass("java/lang/Exception"), "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}
