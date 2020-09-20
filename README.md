## 1.功能概述：
利用现有的STC-B学习板设备，融合结合数码管滚动显示，流水灯功能，电子音乐功能，导航键功能和蜂鸣器功能，485通信功能，将3x3的棋盘滚动地显示在一行地数码管上，实现井字棋单机小游戏和双人对战功能。
在数码管中央棋盘的一行，第一颗数码管显示行号，第二颗和第8颗数码管显示对战双方A或B，3和7颗数码管显示棋盘左右边界，456三颗数码管显示3x3棋盘。通过导航键可以上下滚动查看井字棋任意行，上下左右移动闪烁光标确定位置，按下则落子下棋。

## 2.使用说明
①数码管布局：

- 在数码管中央棋盘的一行，第一颗数码管显示行号，第二颗和第8颗数码管显示对战双方A或B，3和7颗数码管显示棋盘左右边界，456三颗数码管显示3x3棋盘。通过导航键可以上下滚动查看井字棋任意行，上下左右移动闪烁光标确定位置，按下则落子下棋。

②游戏开始：

-    游戏开始前，播放《超级玛丽》开机音乐， 数码管滚动显示字母《PLAY CHESS》表示游戏名称，K2可以选择玩家A或B，按下K1游戏开始，进入游戏棋盘界面。

③游戏中：

-    通过控制导航键上下滚动选择数码管的任意一行，行号显示在第一个数码管上，操作的玩家信息将会用小数点标注；左右控制光标位置，光标闪烁，显示被选中的位置，按下导航键落子下棋；操作过程配有按键提示音效，并且A，B双方玩家的按键提示音不同，加以区分。

④游戏结束：

-    当棋盘存在三点一线或者平局将会触发游戏结束，数码管闪烁，显示赢家信息，胜利一方播放胜利的音乐，表示游戏胜利，失败一方播放失败的音乐，表示游戏失败。游戏全程按下K2键将会触发得分显示，将双方的得分显示在数码管上。 

⑤双人对战功能：

-    用杜邦线连接两块板子，双方可以用A，B两个符号在棋盘上下棋，两人通过485通信进行数据交互，实现实时双人对战。下棋过程中除查看比分外，所有的操作动作都会被传输到对手，实时更新棋盘。游戏采取三局两胜利制，当三局游戏结束，会自动关闭游戏循环，显示滚动字样《PLAY CHESS》

## 3.环境要求
-    stc-b学习板 x 2
-    杜邦线 x 2