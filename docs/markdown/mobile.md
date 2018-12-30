## <a name="mobile-support">Mobile Builds</a>

The GUI makes use of Qt5's support for Android and iOS.

The mobile version of Corluma supports all communication protocols the desktop version supports except serial communication. 

The screen orientation for mobile devices will support portrait and landscape for tablets with an aspect 
ratio of 4:3, such as an iPad. For phones and thinner tablets, only portrait will be supported.

There are a couple of known issues with our current mobile support:

* iOS support is currently experimental.
* When working with iOS and Qt it is generally a good practice to stay up to date with both. For instance, In Qt versions before 5.5.2, there is a compatibility issue with Apple's more recent bitcode setting. 
* Android 7.0 has compatibility issues with QSslSocket, due to dropping support for openssl in favor boringssl. Either recompile Qt from source to not support SSL, or compile your own version of libssl.so and libcrypto.so. To do the latter, check the [Qt Documentation](http://doc.qt.io/qt-5/opensslsupport.html).

