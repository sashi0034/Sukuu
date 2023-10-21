import os
import pyperclip


def generate_constexpr(dir, ext, regisetr_func):
    directory = os.path.dirname(os.path.realpath(__file__)) + "/" + dir
    files = [f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))]

    png_files = [f for f in files if f.endswith(ext)]

    output_text = ""
    for png_file in png_files:
        var_name = png_file.replace(ext, "")
        output_text += f'constexpr StringView {var_name} = U"asset/{dir}/{png_file}";\n'

    output_text += "inline void RegisterAll(){\n"
    for png_file in png_files:
        var_name = png_file.replace(ext, "")
        output_text += f'{regisetr_func}({var_name}, {var_name});\n'
    output_text += "}\n"
    return output_text


if __name__ == "__main__":
    output_text = "#pragma once\n\n"

    output_text += "namespace AssetImages{\n"
    output_text += generate_constexpr("image", ".png", "TextureAsset::Register")
    output_text += "}\n"

    print(output_text)
    pyperclip.copy(output_text)
