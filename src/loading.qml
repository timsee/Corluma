import QtQuick 2.0


Item {
    AnimatedImage {
        id: animation;
        source: "qrc:/images/gif/light_search.gif"
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        fillMode: Image.PreserveAspectFit
    }

// TODO: lottieanimation comes back to Qt in Qt 6.1
//    LottieAnimation {
//        id: animation
//        quality: LottieAnimation.MediumQuality
//        source: "qrc:/images/svg/syncing.json"
//        autoPlay: true
//        onStatusChanged: {
//            if (status === LottieAnimation.Ready) {
//                // any acvities needed before
//                // playing starts go here
//                gotoAndPlay(startFrame);
//            }
//        }
//        onFinished: {
//            console.log("Finished playing")
//        }
//    }
}
