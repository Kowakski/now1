

//---------------------------------【头文件、命名空间包含部分】-----------------------------
//      描述：包含程序所使用的头文件和命名空间
//-------------------------------------------------------------------------------------------------
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;
using namespace std;

//-----------------------------------【宏定义部分】--------------------------------------------
//  描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define WINDOW_NAME "autofocus"        //为窗口标题定义的宏


//-----------------------------------【全局函数声明部分】------------------------------------
//      描述：全局函数的声明
//------------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param);
void DrawRectangle( cv::Mat& img, cv::Rect box );

//-----------------------------------【全局变量声明部分】-----------------------------------
//      描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
Rect g_rectangle;
bool g_bDrawingBox = false;//是否进行绘制
RNG g_rng(12345);
bool contFlag = false; //already ,go on reading

std::string get_img_path( const string dircPath, const int curNum, const int width, const string sub ){
    int curNumDepth, divTmp;
    string rlt;
    string zeros;
    curNumDepth = 1;
    divTmp = curNum;
    while( divTmp/10 > 0 ){
        curNumDepth++;
        divTmp /= 10;
    }

    //how many zeros
    for( int i = 0; i < width - curNumDepth; i++ ){
        zeros += "0";
    }

    rlt = dircPath + "img/" + zeros + std::to_string( curNum ) + "."+sub;
    // cout << "path is " << rlt << endl;
    return rlt;
}

void help( char* argv[] ){
    cout << endl;
    cout<< "Usage tips:"<<endl;
    cout << argv[0] << " /home/sln/share/datas/Walking2/"<< endl;
    cout << endl;
    return;
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c){
  std::string::size_type pos1, pos2;
  if( s.length() <= 0 ) return;
  pos2 = s.find(c);
  pos1 = 0;
  v.clear();
  while(std::string::npos != pos2){
    v.push_back(s.substr(pos1, pos2-pos1));
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

//-----------------------------------【main( )函数】--------------------------------------------
//      描述：控制台应用程序的入口函数，我们的程序从这里开始执行
//-------------------------------------------------------------------------------------------------
int main( int argc, char* argv[] ){
    //【0】改变console字体颜色
    // system("color 9F");
    if( argc != 2 ){
        help( argv );
        return 0;
    }

    int imgNum = 1;
    ifstream label;
    string imgPath;
    string direcPath = argv[1];
    contFlag = false;
    Size subPixWinSize(10,10), winSize(31,31);
    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);

    string stringitem;
    vector< string > tmp;

    namedWindow( WINDOW_NAME, WINDOW_AUTOSIZE );
    //read first one
    imgPath = get_img_path( direcPath, imgNum, 4, "jpg" );
    cv::Mat srcImg = cv::imread( imgPath, IMREAD_COLOR );
    if( srcImg.empty() ){
        cout<<"read error!"<<endl;
        exit(1);
    }
    cout << "image size:" << srcImg.cols << "x" << srcImg.rows << endl;

    label.open( (direcPath + "groundtruth_rect.txt").c_str() );
    getline( label, stringitem, '\n' );
    stringitem.pop_back();
    SplitString( stringitem, tmp, "\t" );
    cout << "label is: " << tmp[0] <<" "<< tmp[1] <<" "<< tmp[2]<<" "<< tmp[3] << endl;
    rectangle( srcImg, Point( stoi(tmp[0]), stoi(tmp[1]) ), Point( stoi(tmp[0])+stoi(tmp[2]), stoi(tmp[1])+stoi(tmp[3]) ), Scalar(0,0,255), 1, LINE_8, 0 );
    cv::Mat ImgShow; //ImgShow used to show, srcImg used to algorithm
    srcImg.copyTo( ImgShow );
    cv::imshow( WINDOW_NAME, srcImg );

    //【1】准备参数
    g_rectangle = Rect(-1,-1,0,0);
    setMouseCallback( WINDOW_NAME, on_MouseHandle, (void*)&srcImg);

    while( 1 ){
        waitKey(10);
        if( contFlag ){
            cout << " Draw Box: " << "( " << g_rectangle.x << "," << g_rectangle.y << " )" << "("<< g_rectangle.x + g_rectangle.width  << "," << g_rectangle.y + g_rectangle.height << ")" << endl;
            break;
        }
        srcImg.copyTo( ImgShow );
        if( g_bDrawingBox ) DrawRectangle( ImgShow, g_rectangle );
        imshow( WINDOW_NAME, ImgShow );
        continue;
    }

    //模板，感兴趣区间获取角点
    Mat keyPointMask( srcImg.rows, srcImg.cols, CV_8UC1, Scalar_<uchar>(0) );
    std::vector<Point2f> keyPoints[2];

    // cout << "line "<< __LINE__ << endl;

    for( int i = g_rectangle.x; i < g_rectangle.x + g_rectangle.width; i++ ){
        for( int j = g_rectangle.y; j < g_rectangle.y + g_rectangle.height; j++ ){
            keyPointMask.at < uchar > (j,i) = 200;   //row first and the cols, reverse with x y
        }
    }
    rectangle( keyPointMask, Point(g_rectangle.x, g_rectangle.y ), Point(g_rectangle.x + g_rectangle.width, g_rectangle.y + g_rectangle.height ), Scalar(255, 255, 255), 1, LINE_8, 0 );
    DrawRectangle( keyPointMask, g_rectangle );
    imshow("MASK", keyPointMask);
    Mat gray, preGray;
    cvtColor( srcImg, gray, COLOR_BGR2GRAY );

    // cout << "Befor get good features: " << gray.cols << " " << gray.rows << "mask:" << keyPointMask.cols << " "<<keyPointMask.rows << endl;
    goodFeaturesToTrack( gray, keyPoints[0], 500, 0.005, 10, keyPointMask, 3, 3, 0, 0.04 );
    cornerSubPix( gray, keyPoints[0], subPixWinSize, Size(-1,-1), termcrit );
    // cout << "After get good features" << endl;
    cout << "key points:";
    for( int i = 0; i < keyPoints[0].size(); i++ ){
        cout << "(" << keyPoints[0][i]<<")" << " ";
        circle( ImgShow, keyPoints[0][i], 3, Scalar(0, 255, 0), -1, 8 );
    }
    cout << endl;

    imshow( WINDOW_NAME, ImgShow ); //显示角点

#if 1
    while(1){        //显示，按下 c 键继续
        if( waitKey(30) == 'c' ) break;
        continue;
    }
#endif

    gray.copyTo( preGray );

    //tracking begin
    while(1){
        imgPath = get_img_path( direcPath, imgNum, 4, "jpg" );
        srcImg = cv::imread( imgPath, IMREAD_COLOR );
        imgNum++;
        // cout << "line: "<<__LINE__<<endl;
        if( srcImg.empty() ){
            cout << "img empty" << endl;
            break;
        }

        getline( label, stringitem, '\n' );
        if( stringitem.length() <= 0 ){
            cout << "label read fail"<<endl;
            break;
        }
        stringitem.pop_back();
        SplitString( stringitem, tmp, "\t" );
        // cout << tmp[0] <<" "<< tmp[1] <<" "<< tmp[2]<<" "<< tmp[3] << endl;
        // cout << "line: "<<__LINE__<<endl;

        // cout << "key 0: "  << keyPoints[0].size() << endl;
        if( keyPoints[0].size() > 0 ){
            vector<uchar> status;
            vector<float> err;

            cvtColor( srcImg, gray, COLOR_BGR2GRAY );
            calcOpticalFlowPyrLK( preGray, gray, keyPoints[0], keyPoints[1], status, err, winSize, 3, termcrit, 0, 0.001  );
            // cout << "In trk :" << keyPoints[0].size() << " " << keyPoints[1].size() << endl;

            for( int i = 0; i < keyPoints[1].size(); i++ ){
                circle( srcImg, keyPoints[1][i], 3, Scalar(255, 0, 0), -1, 8 );
            }
        }
        // cout << "line: "<<__LINE__<<endl;
        cv::swap( preGray, gray );
        std::swap( keyPoints[0], keyPoints[1] );

        // cout << "line: "<<__LINE__<<endl;
        rectangle( srcImg, Point( stoi(tmp[0]), stoi(tmp[1]) ), Point( stoi(tmp[0])+stoi(tmp[2]), stoi(tmp[1])+stoi(tmp[3]) ), Scalar(0,0,255), 1, LINE_8, 0 );
        cv::imshow( WINDOW_NAME, srcImg );
        // cout << "line: "<<__LINE__<<endl;
        if( waitKey( 30 ) == 27 ) break;//按下ESC键，程序退出
    }
    label.close();
    return 0;
}



//--------------------------------【on_MouseHandle( )函数】-----------------------------
//      描述：实时返回当前状态：按下、拖动、弹起来，xy表示当前xy的位置
//-----------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param){

    Mat& image = *(cv::Mat*) param;
    switch( event){
        //鼠标移动消息
        case EVENT_MOUSEMOVE:
            {
                if( g_bDrawingBox ){//如果是否进行绘制的标识符为真，则记录下长和宽到RECT型变量中
                    g_rectangle.width = x-g_rectangle.x;
                    g_rectangle.height = y-g_rectangle.y;
                    // cout << g_rectangle.x << ","<<g_rectangle.y << ":" <<x<<","<<y << endl;
                    // cout << g_rectangle.width << ","<<g_rectangle.height << endl;
                }
            }
        break;

            //左键按下消息
        case EVENT_LBUTTONDOWN:
            {
                g_bDrawingBox = true;
                g_rectangle =Rect( x, y, 0, 0 );//记录起始点
            }
        break;

            //左键抬起消息
        case EVENT_LBUTTONUP:
            {
                g_bDrawingBox = false;//置标识符为false
                contFlag = true;
                //对宽和高小于0的处理
                if( g_rectangle.width < 0 ){  //这种情况防止从右下角往左上角拖
                    g_rectangle.x += g_rectangle.width;
                    g_rectangle.width *= -1;
                }

                if( g_rectangle.height < 0 ){
                    g_rectangle.y += g_rectangle.height;
                    g_rectangle.height *= -1;
                }
                //调用函数进行绘制
                DrawRectangle( image, g_rectangle );
            }
        break;
    }
}

//-----------------------------------【DrawRectangle( )函数】------------------------------
//      描述：自定义的矩形绘制函数
//! the top-left corner
//Point_<_Tp> tl() const;
//! the bottom-right corner
//Point_<_Tp> br() const;

//! returns uniformly distributed double-precision floating-point random number from [a,b) range
//double uniform(double a, double b);
//-----------------------------------------------------------------------------------------------
void DrawRectangle( cv::Mat& img, cv::Rect box ){
    cv::rectangle(img, box.tl(), box.br(), cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255)));//随机颜色
}