# LabelingMachine
基于Qt5的目标识别数据集标签制作程序

## 从视频分解图片集

先使用"文件"->"分解视频文件"打开视频分解工具或者使用快捷键Ctrl+K打开

选择视频文件和图片输出目录,抽取率为每多少帧抽取1张图片,点击确定开始,等待完成


## 从图片集创建工程

使用"文件"->"从图片集创建工程",先选择图片集所在文件夹,(注意:该文件夹内只能包含文件夹和图片文件,不得包含其他文件)

然后选择存放每张图片标记信息的xml文件输出位置

如果弹出发现Project.xml说明先前文件夹内有旧工程,是则加载旧工程的标签并重新生成图片列表,否则生成全新工程

无论使用哪种方式,已经创建的标签文件不会覆盖.

程序会在图片集文件夹下创建名为Project的文件夹,内含Project.xml工程文件


## 打开工程
使用"文件"->"打开工程"或者使用快捷键Ctrl+O,选择Project文件夹下的Project.xml工程文件

## 保存工程
使用"文件"->"保存工程"或者使用快捷键Ctrl+S保存(实际上都是自动保存,所以这个没什么用)

## 界面说明

左侧为图片列表,下拉框为选择显示全部图片,已标记图片,未标记图片

中间为显示区域

右侧上方为类列表,点击编辑类列表弹出编辑框,可以添加删除或修改标签

右侧中间有上一张,下一张按钮,用于切换图片

右侧下方为标签列表,已经标记的标签在这里列出


## 操作说明

点击类列表的行可以切换选中的类

鼠标左键拖动画标签

鼠标滚轮缩放

鼠标中间拖动可拖拽图片

鼠标右键单击标签框内可删除鼠标所在位置的最上层标签框


## 快捷键说明

Q:上一张

E:下一张

R:删除最后的标签

数字键0~9:快速切换选中的类(为了方便'~'键和0效果相同)


## 协作模式说明

主机模式:当打开工程后有效,弹出服务器窗口.左侧为客户端列表,右侧为日志

当客户端全部接入后点击开始会在没有标签的图片中进行任务分配并下发到客户端

然后主机的图片列表也会变成分配的任务列表

客户端模式:只在未打开工程时有效,在窗口中输入主机IP后链接,等待主机开.客户端掉线可重连,只要主机不关闭就可以继续


## 转换说明

选择自己编写的转换程序,LeabelingMachine会为其创建子进程,并附带6个参数

--ProjectPath ${ProjectPath} --XmlPath ${XmlPath} --ImgPath ${ImgPath}

该选项仅在打开本地工程后有效


## Xml格式说明
### Project.xml
```xml
<LablingMachineProject>
          <Project>
              <ImgCount>图片总数</ImgCount>
              <ImgPath>图片集文件夹路径</ImgPath>
              <XmlPath>图片xml输出路径</XmlPath>
          </Project>
          <Labels>
              <label>
                  <ID>类ID</ID>
                  <Name>类的名字</Name>
              </label>
              <label>
              ...可以有多个
              </label>
          </Labels>
          <Image>
              <ImageFilename>图片文件名</ImageFilename>
              <XmlFilename>图片xml文件名</XmlFilename>
          </Image>
          <Image>
          ...可以有多个
          </Image>
      </LablingMachineProject>
```


### 图片xml
```xml
<annotation>
    <filename>图片文件名</filename>
    <size>
        <width>图片宽</width>
        <height>图片高</height>
    </size>
    <object>
        <name>标签的名字</name>
        <ID>标签的ID</ID>
        <bndbox>
            <xmin>标签左上角x坐标</xmin>
            <ymin>标签左上角y坐标</ymin>
            <xmax>标签右下角x坐标</xmax>
            <ymax>标签右下角y坐标</ymax>
        </bndbox>
    </object>
    <object>
    ...
    </object>
</annotation>
```
