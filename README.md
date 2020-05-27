![Build](https://github.com/compilelife/loginsight/workflows/Build/badge.svg)

loginsight致力于打造一款日志分析的利器

![preview](https://gitee.com/compilelife/loginsight/raw/master/doc/img/preview.png)

亮点特性：

- 支持大文件(macbook 2015上2.5G文件5s内打开)
- 时间线功能，日志再乱，思路清晰
- 小窗过滤，双线程查问题
- 快速、轻量、低内存占用
- 切割日志的合并打开（待开发）
- 日志格式化高亮（待开发）
- 智能分析（待开发）



更多贴心细节：

- 搜索框支持历史记录和自动补全
- 匹配结果自动居中在屏幕中央
- 高亮光标下单词
- 主窗口、小窗、时间线高亮联动，思路不断线
- ……



看日志最大的痛点是大文件支持差，用编辑器开几十M的日志都要等半天，几百M基本开不了；所幸VIM还算不错，勉强可以应付，不过VIM会用的毕竟少数。

第二个痛点是，查日志重在找线索，整理思路，发现问题原因，但编辑器远不能胜任。我们需要一个“线索板”时刻帮忙记录、整理思路。“时间线”功能，就是一个初级的“线索板”。

针对上面这些问题/需求，我决定把我用VIM查日志的经验转换为一个工具，并在这个工具的加入更多辅助查日志的好用功能。（更智能一些，是不是可以让工具帮忙找到特定的出错模式呢）



如果觉得这个项目可能对你会有帮助，请支持我：

1. 可以在issue里提需求
2. 可以在issue里针给TODO列表中标`*`的项提实现方案
3. 点击右上方的"Star"
4. 给开发者点杯咖啡，加个油（赞赏时可以在备注提需求，所提需求将被优先实现）：

![support](https://gitee.com/compilelife/loginsight/raw/master/doc/img/support.png)

TODO:

- [x] 基础功能完善和优化
- [x] 支持2G以下文件快速打开
- [ ] 时间线支持自定义格式信息提取（提取日期、进程、TAG等）
- [ ] 日志格式化高亮
- [ ] `*`自动分析
- [ ] `*`流式日志支持（如adb logcat）（待定）



PS：

如果你在GitHub上看不到图片、速度慢；可以移步到镜像仓库：https://gitee.com/compilelife/loginsight

如果在release页面下载速度慢，可以移步百度网盘下载:

链接: https://pan.baidu.com/s/1DzKTmmVL34qQdHKlrs8LCg 提取码: x46v
