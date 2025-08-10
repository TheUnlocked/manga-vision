package com.github.theunlocked.mangavision

import android.graphics.Bitmap

object SpreadDetector {
    @JvmStatic
    fun init() {
        System.loadLibrary("mangavision")
    }

    @JvmStatic
    fun isSpread(left: Bitmap, right: Bitmap): Boolean {
        // The images should have the same height
        if (left.height != right.height) {
            return false
        }

        // Already wide images cannot form a spread
        if (left.width > left.height || right.width > right.height) {
            return false
        }

        // Smaller images could form a spread in theory, but the detection algorithm has not been tested on smaller
        // images, and it may not be reliable if the image is not at least as large as the processing resolution.
        if (left.height < 512 || right.height < 512) {
            return false
        }

        return nIsSpread(left, right);
    }

    @JvmStatic
    private external fun nIsSpread(left: Bitmap, right: Bitmap): Boolean
}