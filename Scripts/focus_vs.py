import pygetwindow as gw
import pyautogui
import time

if __name__ == '__main__':
    # Visual Studioのウィンドウを取得
    windows = gw.getWindowsWithTitle('Visual Studio')

    if windows:
        # 最初のウィンドウにフォーカスを移動
        visual_studio_window = windows[0]
        visual_studio_window.activate()

        # 少し待ってからF5キーを送信
        time.sleep(1)
        pyautogui.press('f5')
    else:
        print("Visual Studio のウィンドウが見つかりませんでした。")
