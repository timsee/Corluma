# Deployments

Corluma can be built from source for Linux, macOS, Windows, iOS, and Android. To deploy to each environment, additional steps are ran after building to package for that environment. `deploy.py`  takes a `.json` file as input, and uses that to determine which environments to build and deploy for release. In all cases, this tool outputs unsigned applications.

Usage example:

```
python3 deploy.py -json_file ./deployment_settings.json
```

For each environment, a json object defines the basic build steps:
```
"linux_deploy": {
    "clean_build": "/usr/bin/make clean -j12",
    "run_qmake": "/path/to/qmake arg1 arg2 arg3",
    "run_make": "/usr/bin/make -j12",
    "qt_build_dir": "/path/to/qt/build/dir"
}
```

The currently supported json objects are `linux_deploy`,  `android_deploy`, `mac_deploy`, `ios_deploy`. Each step can be found from the build outputs of running "Rebuild Project" from Qt Creator.

## <a name="ubuntu">Ubuntu Linux</a>
|  | |
| ----------- | ----------- |
| **Qt Version**  | 6.0.0    |
| **Build Environment**  | Ubuntu   |
| **Output File(s)**  | `.deb`   |

#### Notes
- Creates a `.deb` file.
- A static build of Qt is required.
- As part of your qmake execution, pass `BUILD_STATIC_CORLUMA=1`.
- Qt 6.0.0 removed `QtSerialPort`. To support builds with serial ports, use an earlier version of Qt. The earliest version currently tested is Qt 5.12.5.

## <a name="android">Android</a>
|  | |
| ----------- | ----------- |
| **Qt Version**  | 5.15.0  |
| **Build Environment**  | Ubuntu, macOS, or Windows   |
| **Output File(s)**  | `.aab`, `.apk`   |

#### Notes
- Qt 6.0.0 does not support `QtAndroidExtras`, because of this, earlier version of android are preferred. However, there are build bugs with certain versions of android in Qt related to QtNetwork, so the referenced version of Qt is the suggested build version.
- As part of your qmake execution, pass `BUILD_STATIC_CORLUMA=1`.
- As part of your qmake execution, add a  `OPENSSL_PATH` variable, which points to openssl.pri in this [repo](https://github.com/KDAB/android_openssl). This will add openSSL support which will aid in discovery of Hues.
- `.apk` is best for sharing across devices.
- To sign, use `jarsigner`.

## <a name="mac">macOS</a>
|  | |
| ----------- | ----------- |
| **Qt Version**  | 6.0.0   |
| **Build Environment**  | macOS   |
| **Output File(s)**  | `.dmg`   |

#### Notes

- Creates a `.dmg` file.
- In order to generate a `.dmg`, there exists a dependency on [appdmg](https://github.com/LinusU/node-appdmg). Install following the instructions on its Github README.md.
- A static build of Qt is required.
- As part of your qmake execution, pass `BUILD_STATIC_CORLUMA=1`.
- Qt 6.0.0 removed `QtSerialPort`. To support builds with serial ports, use an earlier version of Qt. The earliest version currently tested is Qt 5.12.5.

## <a name="iOS">iOS</a>
|  | |
| ----------- | ----------- |
| **Qt Version**  | 5.15.1   |
| **Build Environment**  | macOS   |
| **Output File(s)**  | `.xcarchive`  |

- Qt 6.0.0 requires further debugging, it is currently very laggy.
- leave the `run_make` step for ios builds blank, as creating an archive rebuilds from scratch.
- `.xcarchive` is a temporary output object, while we bring online app signing.
- To sign, use Xcode.

## <a name="debian">Windows</a>

#### Notes
- NYI

## <a name="docs">Docs</a>

- Doxygen is a required dependency.
- Rather than passing a full json object, `docs_deploy`  in your json expects a `boolean`. If `true`, docs are deployed, if `false`, docs are skipped.
