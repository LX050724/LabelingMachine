#ifdef UNICODE
#error "Can not use UNICODE！"
#endif

#include <iostream>
#include <fstream>
#include <tinyxml2.h>
#include "ArgResolver.h"

#ifdef linux

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#else

#include <windows.h>
#include <time.h>
#include <direct.h>
#include <io.h>

#endif
using namespace std;
using namespace tinyxml2;

string ImageProcess(const char *XmlFilename);

bool FileCopy(const char *Src, const char *Dst);

bool CreateDir(const char *Path);

string XmlPath;
string ImgPath;
string ProjectPath;
string OutputPath;

int main(int argc, char *argv[]) {
    if (argc < 9) {
        cerr << "error: No Input" << endl;
        return -1;
    }

    ArgResolver Args(argc, argv, true);

    XmlPath = Args.findArg("XmlPath").getStrVal();
    ImgPath = Args.findArg("ImgPath").getStrVal();
    ProjectPath = Args.findArg("ProjectPath").getStrVal();
    OutputPath = Args.findArg("OutputPath").getStrVal();

    CreateDir((OutputPath + "/ImageSets").c_str());
    CreateDir((OutputPath + "/JPEGImages").c_str());
    CreateDir((OutputPath + "/labels").c_str());
    CreateDir((OutputPath + "/weigits").c_str());
    CreateDir((OutputPath + "/backup").c_str());

    ofstream log;
    log.open(OutputPath + "/YOLOV3module.log");

    streambuf *streams = cout.rdbuf();//保存原来的cout对象
    cout.rdbuf(log.rdbuf());
    Args.printAll();
    cout.rdbuf(streams);

    tinyxml2::XMLDocument ProjectXml;
    ProjectXml.LoadFile(ProjectPath.c_str());

    XMLElement *root = ProjectXml.FirstChildElement("LablingMachineProject");
    XMLElement *Labels = root->FirstChildElement("Labels");
    XMLElement *label = Labels->FirstChildElement("label");

    int classconut = 0;
    if (label) {
        ofstream vocnames;
        vocnames.open(OutputPath + "/voc.names");
        do {
            if (strcmp(label->Name(), "label") == 0) {
                int ID = label->FirstChildElement("ID")->IntText(0);
                string Name(label->FirstChildElement("Name")->GetText());
                cout << "Class ID=" << ID << " Name=" << Name << endl;
                log << "Class ID=" << ID << " Name=" << Name << endl;
                vocnames << Name << endl;
                ++classconut;
            }
        } while ((label = label->NextSiblingElement()) != nullptr);
        vocnames.close();

        ofstream vocdata;
        vocdata.open(OutputPath + "/voc.data");
        vocdata << "classes= " << classconut << endl;
        vocdata << "train  = " << OutputPath << "/ImageSets/train.txt" << endl;
        vocdata << "valid  = " << OutputPath << "/ImageSets/val.txt" << endl;
        vocdata << "names  = " << OutputPath << "/voc.names" << endl;
        vocdata << "backup = " << OutputPath << "/backup" << endl;
        vocdata.close();
    }

    vector<string> imglist;
    XMLElement *Image = root->FirstChildElement("Image");
    if (Image) {
        do {
            if (strcmp(Image->Name(), "Image") == 0) {
                string filename = ImageProcess(Image->FirstChildElement("XmlFilename")->GetText());
                if (filename.empty()) continue;
                cout << filename << " complete!" << endl;
                log << filename << " complete!" << endl;
                imglist.push_back(OutputPath + "/JPEGImages/" + filename);
            }
        } while ((Image = Image->NextSiblingElement()) != nullptr);
    }

    srand(time(NULL));
    cout << "Class ToTal:" << classconut << endl << "Photo Total:" << imglist.size() << endl;
    log << "Class ToTal:" << classconut << endl << "Photo Total:" << imglist.size() << endl;

    ofstream test, train, val;
    int testcount = 0, traincount = 0, valcount = 0;
    test.open(OutputPath + "/ImageSets/test.txt");
    train.open(OutputPath + "/ImageSets/train.txt");
    val.open(OutputPath + "/ImageSets/val.txt");

    for (const string &img : imglist) {
        int n = rand() % 100;
        if (n <= 85) {
            train << img << endl;
            ++traincount;
        } else if (n <= 95 && n > 85) {
            val << img << endl;
            ++valcount;
        } else {
            test << img << endl;
            ++testcount;
        }
    }
    cout << "train:" << traincount << endl;
    cout << "val:" << valcount << endl;
    cout << "test:" << testcount << endl;
    log << "train:" << traincount << endl;
    log << "val:" << valcount << endl;
    log << "test:" << testcount << endl;

    cout << "filters=" << 3 * (classconut + 5) << endl;
    log << "filters=" << 3 * (classconut + 5) << endl;

    test.close();
    train.close();
    val.close();
    log.close();
}

string ImageProcess(const char *XmlFilename) {
    tinyxml2::XMLDocument ImgXml;
    ofstream file;

    ImgXml.LoadFile((XmlPath + '/' + XmlFilename).c_str());

    XMLElement *root = ImgXml.FirstChildElement("annotation");
    XMLElement *size = root->FirstChildElement("size");

    string filename(root->FirstChildElement("filename")->GetText());
    double width = size->FirstChildElement("width")->IntText();
    double height = size->FirstChildElement("height")->IntText();

    XMLElement *object = root->FirstChildElement("object");

    if (object) {
        string labelfilename = filename.substr(0, filename.find_last_of('.')) + ".txt";
        file.open(OutputPath + "/labels/" + labelfilename);
        do {
            if (strcmp(object->Name(), "object") == 0) {
                XMLElement *bandbox = object->FirstChildElement("bandbox");
                string name(object->FirstChildElement("name")->GetText());
                int ID = object->FirstChildElement("ID")->IntText();
                double xmin = bandbox->FirstChildElement("xmin")->IntText() / width;
                double ymin = bandbox->FirstChildElement("ymin")->IntText() / height;
                double xmax = bandbox->FirstChildElement("xmax")->IntText() / width;
                double ymax = bandbox->FirstChildElement("ymax")->IntText() / height;
                double midx = (xmin + xmax) / 2;
                double midy = (ymin + ymax) / 2;
                double boxwidth = xmax - xmin;
                double boxheight = ymax - ymin;
                file << ID << ' ' << midx << ' ' << midy << ' ' << boxwidth << ' ' << boxheight << endl;
            }
        } while ((object = object->NextSiblingElement()) != nullptr);
        file.close();
        FileCopy((ImgPath + '/' + filename).c_str(),
                 (OutputPath + "/JPEGImages/" + filename).c_str());
    } else return string();

    return filename;
}

#ifdef linux

bool FileCopy(const char *Src, const char *Dst) {
    int fd_in, fd_out;
    struct stat stat{};
    loff_t len, ret;
    fd_in = open(Src, O_RDONLY);
    if (fd_in == -1)
        return false;

    if (fstat(fd_in, &stat) == -1) {
        close(fd_in);
        return false;
    }
    len = stat.st_size;
    fd_out = open(Dst, O_CREAT | O_WRONLY, 0777);
    if (fd_out == -1)
        return false;

    do {
        ret = copy_file_range(fd_in, NULL, fd_out, NULL, len, 0);
        if (ret == -1) {
            close(fd_in);
            close(fd_out);
            return false;
        }
        len -= ret;
    } while (len > 0 && ret > 0);

    close(fd_in);
    close(fd_out);
    return true;
}

bool CreateDir(const char *Path) {
    DIR *tmp = opendir(Path);
    if (tmp) closedir(tmp);
    else if (mkdir(Path, 0777) < 0)
        return false;
    return true;
}

#else

bool FileCopy(const char *Src, const char *Dst) {
    return CopyFile(Src, Dst, false);
}

bool CreateDir(const char *Path) {
    if (_access(Path, _A_NORMAL) == 0)
        return true;
    return _mkdir(Path) == 0;
}

#endif