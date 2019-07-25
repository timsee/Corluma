/*!
 * MIT License
 *
 * Copyright (c) 2019 Tim Seemann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#include <jni.h>

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QtAndroidExtras/QAndroidJniObject>

#include "shareutils.hpp"

AndroidShareUtils* AndroidShareUtils::mInstance = NULL;

AndroidShareUtils::AndroidShareUtils(QObject* parent) : PlatformShareUtils(parent) {
    // we need the instance for JNI Call
    mInstance = this;
}

AndroidShareUtils* AndroidShareUtils::getInstance() {
    if (!mInstance) {
        mInstance = new AndroidShareUtils;
    }

    return mInstance;
}

bool AndroidShareUtils::checkMimeTypeView(const QString& mimeType) {
    QAndroidJniObject jsMime = QAndroidJniObject::fromString(mimeType);
    jboolean verified
        = QAndroidJniObject::callStaticMethod<jboolean>("org/shareluma/utils/QShareUtils",
                                                        "checkMimeTypeView",
                                                        "(Ljava/lang/String;)Z",
                                                        jsMime.object<jstring>());
    return verified;
}

/*
 * If a requestId was set we want to get the Activity Result back (recommended)
 * We need the Request Id and Result Id to control our workflow
 */
void AndroidShareUtils::sendFile(const QString& filePath,
                                 const QString& title,
                                 const QString& mimeType,
                                 int requestId) {
    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsTitle = QAndroidJniObject::fromString(title);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/shareluma/utils/QShareUtils",
        "sendFile",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
        jsPath.object<jstring>(),
        jsTitle.object<jstring>(),
        jsMimeType.object<jstring>(),
        requestId);
    if (!ok) {
        qWarning() << "Unable to resolve activity from Java";
        emit shareNoAppAvailable(requestId);
    }
}

/*
 * If a requestId was set we want to get the Activity Result back (recommended)
 * We need the Request Id and Result Id to control our workflow
 */
void AndroidShareUtils::viewFile(const QString& filePath,
                                 const QString& title,
                                 const QString& mimeType,
                                 int requestId) {
    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsTitle = QAndroidJniObject::fromString(title);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/shareluma/utils/QShareUtils",
        "viewFile",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
        jsPath.object<jstring>(),
        jsTitle.object<jstring>(),
        jsMimeType.object<jstring>(),
        requestId);
    if (!ok) {
        qWarning() << "Unable to resolve activity from Java";
        emit shareNoAppAvailable(requestId);
    }
}

// used from QAndroidActivityResultReceiver
void AndroidShareUtils::handleActivityResult(int receiverRequestCode,
                                             int resultCode,
                                             const QAndroidJniObject&) {
    //    qDebug() << "From JNI QAndroidActivityResultReceiver: " << receiverRequestCode
    //             << "ResultCode:" << resultCode;
    processActivityResult(receiverRequestCode, resultCode);
}

// used from Activity.java onActivityResult()
void AndroidShareUtils::onActivityResult(int requestCode, int resultCode) {
    //    qDebug() << "From Java Activity onActivityResult: " << requestCode
    //             << "ResultCode:" << resultCode;
    processActivityResult(requestCode, resultCode);
}

void AndroidShareUtils::processActivityResult(int requestCode, int resultCode) {
    const int RESULT_OK = -1;
    const int RESULT_CANCELED = 0;

    // we're getting RESULT_OK only if edit is done
    if (resultCode == RESULT_OK) {
        //   emit shareEditDone(requestCode);
    } else if (resultCode == RESULT_CANCELED) {
        emit shareFinished(requestCode);
    } else {
        qDebug() << "wrong result code: " << resultCode << " from request: " << requestCode;
        emit shareError(requestCode, tr("Share: an Error occured"));
    }
}

void AndroidShareUtils::checkPendingIntents(const QString& workingDirPath) {
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        // create a Java String for the Working Dir Path
        QAndroidJniObject jniWorkingDir = QAndroidJniObject::fromString(workingDirPath);
        if (!jniWorkingDir.isValid()) {
            qWarning() << "QAndroidJniObject jniWorkingDir not valid.";
            emit shareError(0, tr("Share: an Error occured\nWorkingDir not valid"));
            return;
        }
        activity.callMethod<void>(
            "checkPendingIntents", "(Ljava/lang/String;)V", jniWorkingDir.object<jstring>());
        qDebug() << "checkPendingIntents: " << workingDirPath;
        return;
    }
    qDebug() << "checkPendingIntents: Activity not valid";
}

void AndroidShareUtils::setFileUrlReceived(const QString& url) {
    if (url.isEmpty()) {
        qWarning() << "setFileUrlReceived: we got an empty URL";
        emit shareError(0, tr("Empty URL received"));
        return;
    }
    // qDebug() << "AndroidShareUtils setFileUrlReceived: we got the File URL from JAVA: " << url;
    QString myUrl;
    if (url.startsWith("file://")) {
        myUrl = url.right(url.length() - 7);
    } else {
        myUrl = url;
    }

    // check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    if (fileInfo.exists()) {
        emit fileUrlReceived(myUrl);
    } else {
        emit shareError(0, tr("File does not exist: %1").arg(myUrl));
    }
}

void AndroidShareUtils::setFileReceivedAndSaved(const QString& url) {
    if (url.isEmpty()) {
        emit shareError(0, tr("Empty URL received"));
        return;
    }
    // qDebug() << "AndroidShareUtils setFileReceivedAndSaved: we got the File URL from JAVA: " <<
    // url;
    QString myUrl;
    if (url.startsWith("file://")) {
        myUrl = url.right(url.length() - 7);
    } else {
        myUrl = url;
    }

    // check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    if (fileInfo.exists()) {
        emit fileReceivedAndSaved(myUrl);
    } else {
        emit shareError(0, tr("File does not exist: %1").arg(myUrl));
    }
}

// to be safe we check if a File Url from java really exists for Qt
// if not on the Java side we'll try to read the content as Stream
bool AndroidShareUtils::checkFileExits(const QString& url) {
    if (url.isEmpty()) {
        emit shareError(0, tr("Empty URL received"));
        return false;
    }
    // qDebug() << "AndroidShareUtils checkFileExits: we got the File URL from JAVA: " << url;
    QString myUrl;
    if (url.startsWith("file://")) {
        myUrl = url.right(url.length() - 7);
    } else {
        myUrl = url;
    }

    // check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    return fileInfo.exists();
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_org_shareluma_activity_QShareActivity_setFileUrlReceived(JNIEnv* env,
                                                                                     jobject,
                                                                                     jstring url) {
    const char* urlStr = env->GetStringUTFChars(url, nullptr);
    AndroidShareUtils::getInstance()->setFileUrlReceived(urlStr);
    env->ReleaseStringUTFChars(url, urlStr);
    return;
}

JNIEXPORT void JNICALL
Java_org_shareluma_activity_QShareActivity_setFileReceivedAndSaved(JNIEnv* env,
                                                                   jobject,
                                                                   jstring url) {
    const char* urlStr = env->GetStringUTFChars(url, nullptr);
    AndroidShareUtils::getInstance()->setFileReceivedAndSaved(urlStr);
    env->ReleaseStringUTFChars(url, urlStr);
    return;
}

JNIEXPORT bool JNICALL Java_org_shareluma_activity_QShareActivity_checkFileExits(JNIEnv* env,
                                                                                 jobject,
                                                                                 jstring url) {
    const char* urlStr = env->GetStringUTFChars(url, nullptr);
    bool exists = AndroidShareUtils::getInstance()->checkFileExits(urlStr);
    env->ReleaseStringUTFChars(url, urlStr);
    return exists;
}

JNIEXPORT void JNICALL
Java_org_shareluma_activity_QShareActivity_fireActivityResult(JNIEnv*,
                                                              jobject,
                                                              jint requestCode,
                                                              jint resultCode) {
    AndroidShareUtils::getInstance()->onActivityResult(requestCode, resultCode);
    return;
}

#ifdef __cplusplus
}
#endif
