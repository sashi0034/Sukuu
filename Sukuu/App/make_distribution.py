import os
import shutil
import zipfile

target_dir_name = "Mutant"

folders_to_copy = [
    "asset/font",
    "asset/other",
    "asset/shader",
    "dll",
]
files_to_copy = [
    "credit.html",
    "Sukuu.exe"
]


def _rename_file(dest_file, file):
    if file == "Sukuu.exe":
        os.rename(dest_file, os.path.join(target_dir_name, "Mutant.exe"))


def main():
    if os.path.exists(target_dir_name):
        shutil.rmtree(target_dir_name)

    os.makedirs(target_dir_name)

    for folder in folders_to_copy:
        if os.path.exists(folder):
            shutil.copytree(folder, os.path.join(target_dir_name, folder))

    for file in files_to_copy:
        if os.path.exists(file):
            dest_file = os.path.join(target_dir_name, file)
            shutil.copy(file, dest_file)

            _rename_file(dest_file, file)

    with zipfile.ZipFile(target_dir_name + ".zip", 'w') as zipf:
        for foldername, subfolders, filenames in os.walk(target_dir_name):
            for filename in filenames:
                file_path = os.path.join(foldername, filename)
                zipf.write(file_path, os.path.relpath(file_path, os.path.dirname(target_dir_name)))


if __name__ == "__main__":
    main()
