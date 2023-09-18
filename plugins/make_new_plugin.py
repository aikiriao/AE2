"""
テンプレートプラグインから新しいプラグインを作成
"""
import glob
import shutil
import sys
import os

TEMPLATE_PLUGIN_DIRECTORY = "./00_template"
TEMPLATE_PLUGIN_MODULE_NAME = "AE2Template"
EXTENSION_LIST = [".h", ".cpp", ".txt"]

if __name__ == "__main__":
    NEW_PLUGIN_NAME = sys.argv[1]

    # 既存の場合はエラーにする
    if os.path.exists(NEW_PLUGIN_NAME):
        print(f"ERROR: \"{NEW_PLUGIN_NAME}\" is already exists.")
        sys.exit(1)

    # テンプレートからコピー
    shutil.copytree(TEMPLATE_PLUGIN_DIRECTORY, NEW_PLUGIN_NAME)

    # 文字置換

    # 置換対象のファイルをリストに集める
    file_list = []
    for ext in EXTENSION_LIST:
        file_list.extend(glob.glob(NEW_PLUGIN_NAME + "/**/*" + ext, recursive=True))

    # リネーム
    for file_name in file_list:
        with open(file_name, 'r', encoding='utf-8') as f:
            contents = f.read().replace(TEMPLATE_PLUGIN_MODULE_NAME, NEW_PLUGIN_NAME)
        with open(file_name, 'w', encoding='utf-8') as f:
            f.write(contents)
