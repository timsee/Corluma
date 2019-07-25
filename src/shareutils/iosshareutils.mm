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
#import "shareutils.hpp"

#import <UIKit/UIKit.h>
#import <QDesktopServices>
#import <QFileInfo>
#import <QGuiApplication>
#import <QUrl>
#import <QDebug>

#import <UIKit/UIDocumentInteractionController.h>

#import "docviewcontroller.hpp"

@interface DocViewController ()
@end
@implementation DocViewController
#pragma mark -
#pragma mark View Life Cycle
- (void)viewDidLoad {
    [super viewDidLoad];
}
#pragma mark -
#pragma mark Document Interaction Controller Delegate Methods
- (UIViewController *) documentInteractionControllerViewControllerForPreview: (UIDocumentInteractionController *) controller {
#pragma unused (controller)
    return self;
}
- (void)documentInteractionControllerDidEndPreview:(UIDocumentInteractionController *)controller
{
#pragma unused (controller)
    
    self.mIosShareUtils->handleDocumentPreviewDone(self.requestId);
    
    [self removeFromParentViewController];
}
@end




IosShareUtils::IosShareUtils(QObject* parent) : PlatformShareUtils(parent) {
    // Sharing Files from other iOS Apps I got the ideas and some code contribution from:
    // Thomas K. Fischer (@taskfabric) - http://taskfabric.com - thx
    QDesktopServices::setUrlHandler("file", this, "handleFileUrlReceived");
}

bool IosShareUtils::checkMimeTypeView(const QString& mimeType) {
#pragma unused(mimeType)
    // dummi implementation on iOS
    // MimeType not used yet
    return true;
}

// altImpl not used yet on iOS, on Android twi ways to use JNI
void IosShareUtils::sendFile(const QString& filePath,
                             const QString& title,
                             const QString& mimeType,
                             int requestId) {
#pragma unused(title, mimeType)
    
    NSString* nsFilePath = filePath.toNSString();
    NSURL* nsFileUrl = [NSURL fileURLWithPath:nsFilePath];
    
    static DocViewController* docViewController = nil;
    if (docViewController != nil) {
        [docViewController removeFromParentViewController];
        [docViewController release];
    }
    
    UIDocumentInteractionController* documentInteractionController = nil;
    documentInteractionController =
    [UIDocumentInteractionController interactionControllerWithURL:nsFileUrl];
    
    UIViewController* qtUIViewController =
    [[[[UIApplication sharedApplication] windows] firstObject] rootViewController];
    if (qtUIViewController != nil) {
        docViewController = [[DocViewController alloc] init];
        
        docViewController.requestId = requestId;
        // we need this to be able to execute handleDocumentPreviewDone() method,
        // when preview was finished
        docViewController.mIosShareUtils = this;
        
        [qtUIViewController addChildViewController:docViewController];
        documentInteractionController.delegate = docViewController;
        // [documentInteractionController presentPreviewAnimated:YES];
        if (![documentInteractionController presentPreviewAnimated:YES]) {
            emit shareError(0, tr("No App found to open: %1").arg(filePath));
        }
    }
}


void IosShareUtils::viewFile(const QString& filePath,
                             const QString& title,
                             const QString& mimeType,
                             int requestId) {
    sendFile(filePath, title, mimeType, requestId);
}

void IosShareUtils::handleDocumentPreviewDone(const int& requestId) {
    // documentInteractionControllerDidEndPreview
    // qDebug() << "handleShareDone: " << requestId;
    emit shareFinished(requestId);
}

void IosShareUtils::handleFileUrlReceived(const QUrl& url) {
    QString incomingUrl = url.toString();
    if (incomingUrl.isEmpty()) {
        qWarning() << "setFileUrlReceived: we got an empty URL";
        emit shareError(0, tr("Empty URL received"));
        return;
    }
    //qDebug() << "IosShareUtils setFileUrlReceived: we got the File URL from iOS: " << incomingUrl;
    QString myUrl;
    if (incomingUrl.startsWith("file://")) {
        myUrl = incomingUrl.right(incomingUrl.length() - 7);
    } else {
        myUrl = incomingUrl;
    }
    
    // check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    if (fileInfo.exists()) {
        emit fileUrlReceived(myUrl);
    } else {
        emit shareError(0, tr("File does not exist: %1").arg(myUrl));
    }
}
