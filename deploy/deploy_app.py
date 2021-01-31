# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

import argparse
from CorDeployTool import CorDeployTool

def cli_to_args():
    """
    converts the command line interface to a series of args
    """
    cli = argparse.ArgumentParser(description="")
    cli.add_argument('-env', type=str, required=True,
                     choices=["android", "linux", "docs"],
                     help='Environment to build.')
    cli.add_argument('-qt_build_dir',
                     type=str, required=True,
                     help='Qt build dir where the built application will be.')
    return cli.parse_args()


def main():
    args = cli_to_args()
    deploy_tool = CorDeployTool()
    print(f"Deploying Corluma version {deploy_tool.version_str} for environment: {args.env}")
    if args.env == 'linux':
        deploy_tool.run_linux(args.qt_build_dir)
    elif args.env == 'android':
        deploy_tool.run_android(args.qt_build_dir)
    elif args.env == 'docs':
    	deploy_tool.run_docs()


if __name__ == "__main__":
    main()
