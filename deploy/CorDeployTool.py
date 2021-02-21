# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import fileinput
import os
import sys
from shutil import copyfile
from shutil import rmtree
from distutils.dir_util import copy_tree
from pathlib import Path


class CorDeployTool:

    def __init__(self):
        self.version_str = self.get_app_version()
        self.output_dir = f"./output/{self.version_str}"
        Path(f"./output/{self.version_str}").mkdir(parents=True, exist_ok=True)

    def get_app_version(self):
        app_path = "../src/Corluma.pro"
        version_variable = "VERSION = "
        f = open(app_path)
        pro_file = f.read()
        for line in pro_file.split("\n"):
            if version_variable in line:
                line = line.strip()
                line = line.replace(version_variable, '')
                return line
        # exit out if version can't be found.
        sys.exit("Could not find Corluma Version in qmake file.")


    def replace_str_in_place(self, input_file, string_to_replace, new_string):
        with fileinput.FileInput(input_file, inplace=True) as file:
            for line in file:
                print(line.replace(string_to_replace, new_string), end='')

    def run_android(self, qt_build_dir):
        aab_path = f"{qt_build_dir}/android-build/build/outputs/bundle/release/android-build-release.aab"
        output_aab_path = f"{self.output_dir}/Corluma-{self.version_str}.aab"
        apk_path = f"{qt_build_dir}/android-build/build/outputs/apk/debug/android-build-debug.apk"
        output_apk_path = f"{self.output_dir}/Corluma-{self.version_str}.apk"
        print(f"aab_path: {aab_path}")
        os.system(f"cp {aab_path} {output_aab_path}")
        os.system(f"cp {apk_path} {output_apk_path}")


    def run_linux(self, qt_buid_dir):
        # convert version_str to linux standard
        version_list = self.version_str.split('.')
        if len(version_list) != 3:
            print("version_str invalid, cannot deploy linux")
            return
        linux_version_str = f"{version_list[0]}.{version_list[1]}-{version_list[2]}"

        # define parameters
        app_name = "corluma"
        deploy_dir = f"{app_name}_{linux_version_str}"
        deb_file = f"{deploy_dir}.deb"
        source_file = f"{qt_buid_dir}/{app_name}"
        dest_file = f"{deploy_dir}/usr/bin/{app_name}"

        # copy the linux .deb
        copy_tree('linux', deploy_dir)

        # find and replace the version string in deployed files
        self.replace_str_in_place(f"{deploy_dir}/DEBIAN/control",
                                  f"Version: ~~VERSION_STRING~~",
                                  f"Version: {linux_version_str}")
        self.replace_str_in_place(f"{deploy_dir}/usr/share/applications/corluma.desktop",
                                  f"Version=~~VERSION_STRING~~",
                                  f"Version={linux_version_str}")

        # Print file permission of the destination
        #perm = os.stat(source_file).st_mode
        #print(f"File permission before copy: {perm}")

        # copy the corluma binary to the bin directory
        os.system(f"cp -p {source_file} {dest_file}")

        # Print file permission of the destination
        #perm = os.stat(dest_file).st_mode
        #print(f"File permission after copy: {perm}")

        # run dpkg-build
        os.system(f"dpkg-deb --build {deploy_dir}")

        # copy .deb to output directory
        os.system(f"cp -p {deb_file} {self.output_dir}/{deb_file}")

        # remove working dir
        rmtree(deploy_dir)

        # remove original .deb
        os.remove(deb_file)


    def run_mac(self, qt_build_dir):
        # define the path to the app in build and where it'll go
        app_path = f"{qt_build_dir}/Corluma.app"
        output_app_path = f"macOS/Corluma.app"
        final_dmg_path = f"{self.output_dir}/Corluma_{self.version_str}.dmg"
        print(f"app_path: {app_path} output_path: {output_app_path}")
        # copy the app to the working directory
        os.system(f"cp -r {app_path} {output_app_path}")
        # remove the existing dmg, if one already exists
        os.system(f"rm -r {final_dmg_path}")
        # run a script to generate the dmg
        os.system(f"appdmg ./macOS/spec.json {final_dmg_path}")
        # remove the working paths
        os.system(f"rm -r {output_app_path}")


    def run_ios(self, qt_build_dir):
        xcode_proj_path = f"{qt_build_dir}/Corluma.xcodeproj"
        output_file = f"{self.output_dir}/Corluma_{self.version_str}.xcarchive"
        os.system(f"xcodebuild -project {xcode_proj_path} -scheme Corluma -archivePath {output_file} archive")

    def run_docs(self):
        docs_workspace_path = "./workspace_docs"
        docs_output_path = f"{docs_workspace_path}/output"
        latex_path = f"{docs_output_path}/latex"
        doxyfile_path = f"{docs_workspace_path}/Doxyfile_Corluma"
        os.system(f"cp -rp ../docs/ {docs_workspace_path}")
        self.replace_str_in_place(f"{doxyfile_path}",
                                  f"PROJECT_NUMBER = ~~VERSION_STRING~~",
                                  f"PROJECT_NUMBER = v{self.version_str}")
        os.system(f"rm -rf {docs_output_path}")
        os.system(f"mkdir {docs_output_path}")
        # make output directory if it doesn't exist
        Path(f"{self.output_dir}/docs").mkdir(parents=True, exist_ok=True)
        # run doxygen
        os.system(f"doxygen {doxyfile_path}")
        # cleanup doxygen workspace
        os.system(f"rm -rf {docs_workspace_path}")
