## <a name="mobile-support">Mobile Builds</a>

The GUI makes use of Qt5's support for Android and iOS.

The mobile version of Corluma supports all communication protocols the desktop version supports except serial communication. 

There are a couple of known issues with our current mobile support:

* iOS support is currently experimental.
* Android 7.0 has compatibility issues with QSslSocket, due to dropping support for openssl in favor boringssl. Either recompile Qt from source to not support SSL, or compile your own version of libssl.so and libcrypto.so. To do the latter, check the [Qt Documentation](http://doc.qt.io/qt-5/opensslsupport.html).

