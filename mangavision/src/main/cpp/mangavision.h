//
// Created by Unlocked on 8/7/2025.
//

#ifndef MANGA_VISION_MANGAVISION_H
#define MANGA_VISION_MANGAVISION_H

#define CV_STATIC_ANALYSIS

#include <jni.h>
#include <opencv2/core.hpp>

void bitmapToMat(JNIEnv* env, jobject bitmap, cv::Mat &dst);

constexpr int PROCESSING_RESOLUTION = 512;
void preprocess(cv::Mat &mat);

constexpr int QUANTIZATION_LEVELS = 16;
void uniformQuantize(cv::Mat &mat);

constexpr int INTERESTINGNESS_REGION_COUNT = 8;
constexpr int MIN_INTERESTING_REGION_COUNT = 4;
constexpr int MAX_DIFFERING_REGION_COUNT = 1;

std::array<bool, INTERESTINGNESS_REGION_COUNT> getInterestingnessMap(const cv::Mat& mat);

constexpr double CANNY_THRESHOLD_MIN = 15.0;
constexpr double CANNY_THRESHOLD_MAX = 50.0;

constexpr double SEAM_DISCONTINUITY_THRESHOLD = 0.13;

double getSeamDiscontinuity(const cv::Mat& mat1, const cv::Mat& mat2);

#endif //MANGA_VISION_MANGAVISION_H
