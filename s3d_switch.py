import re
import sys
import os

switch_version = "0_6_15"


def update_vcxproj_files(directory, switch_version):
    for root, dirs, files in os.walk(directory):
        for file_name in files:
            if not file_name.endswith(".vcxproj"):
                continue

            file_path = os.path.join(root, file_name)
            with open(file_path, "r", encoding="utf-8") as file:
                file_contents = file.read()

            updated_contents = re.sub(
                r"\$\(SIV3D_\d+_\d+_\d+\)",
                f"$(SIV3D_{switch_version})",
                file_contents,
            )

            with open(file_path, "w", encoding="utf-8") as file:
                file.write(updated_contents)

            print(f"Updated: {file_path}")


if __name__ == "__main__":
    if len(sys.argv) == 2:
        switch_version = sys.argv[1]

    current_directory = os.getcwd()

    update_vcxproj_files(current_directory, switch_version)
