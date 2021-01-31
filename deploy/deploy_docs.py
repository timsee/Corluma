# Copyright 2021 by Tim Seemann
# Released under the GNU General Public License.

from CorDeployTool import CorDeployTool

def main():
    # makes a deploy tool
    deploy_tool = CorDeployTool()
    # deploys docs
    print(f"Deploying docs for {deploy_tool.version_str}")
    deploy_tool.run_docs()

if __name__ == "__main__":
    main()
