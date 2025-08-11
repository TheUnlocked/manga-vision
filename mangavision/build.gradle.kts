import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    alias(libs.plugins.maven.publish)
    alias(libs.plugins.jreleaser)
}

android {
    namespace = "io.github.theunlocked.mangavision"
    compileSdk = 36

    defaultConfig {
        minSdk = 26

        consumerProguardFiles("consumer-rules.pro")
        externalNativeBuild {
            cmake {
                cppFlags += "-frtti -fexceptions"
                arguments += listOf(
                    "-DOpenCV_DIR=${projectDir}/../opencv-sdk/OpenCV-android-sdk/sdk/native/jni",
                    "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"
                )
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    publishing {
        singleVariant("release") {
            withJavadocJar()
            withSourcesJar()
        }
    }
}

kotlin {
    compilerOptions {
        jvmTarget = JvmTarget.fromTarget("17")
    }
}

dependencies {
}

publishing {
    publications {
        val mavenPublishVersion = System.getenv("MAVEN_PUBLISH_VERSION")
        if (mavenPublishVersion != null) {
            register<MavenPublication>("release") {
                groupId = "io.github.theunlocked"
                artifactId = "manga-vision"
                version = mavenPublishVersion

                pom {
                    name = "manga-vision"
                    description = "An android library that uses OpenCV to process manga images"
                    licenses {
                        license {
                            name = "MIT License"
                            url = "https://opensource.org/license/MIT"
                            distribution = "repo"
                        }
                    }
                    scm {
                        url = "https://github.com/TheUnlocked/manga-vision"
                    }
                }

                afterEvaluate {
                    from(components["release"])
                }
            }
        }
    }
    repositories {
        maven {
            url = uri(layout.buildDirectory.dir("staging-deploy").get())
        }
    }
}

jreleaser {
    version = "1.0"
    gitRootSearch = true
    signing {
        setActive("ALWAYS")
        armored = true
    }
    release {
        github {
            skipRelease = true
        }
    }
    deploy {
        maven {
            mavenCentral {
                create("sonatype") {
                    setActive("ALWAYS")
                    url = "https://central.sonatype.com/api/v1/publisher"
                    stagingRepository("build/staging-deploy")
                }
            }
        }
    }
}
