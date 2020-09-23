

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

    for( int i = 0; i < width - curNumDepth; i++ ){
        zeros += "0";
    }

    rlt = dircPath + zeros + std::to_string( curNum ) + "."+sub;
    return rlt;
}

void help( char* argv[] ){
    cout << endl;
    cout<< "Usage tips:"<<endl;
    cout << argv[0] << " /home/sln/share/datas/Walking2/img/"<< endl;
    cout << endl;
    return;
}
//-----------------------------------【main( )函数】--------------------------------------------
//      描述：控制台应用程序的入口函数，我们的程序从这里开始执行
//-------------------------------------------------------------------------------------------------
int main( int argc, char* argv[] ){
    //【0】改变console字体颜色
    system("color 9F");
    if( argc != 2 ){
        help( argv );
        return 0;
    }

    int imgNum = 1;
    string imgPath;
    string direcPath = argv[1];
    contFlag = false;

    namedWindow( WINDOW_NAME, WINDOW_AUTOSIZE );
    //read first one
    imgPath = get_img_path( direcPath, imgNum, 4, "jpg" );
    cv::Mat srcImg = cv::imread( imgPath, IMREAD_COLOR );
    if( srcImg.empty() ){
        cout<<"read error!"<<endl;
        exit(1);
    }
    cv::Mat srcImgtmp;
    srcImg.copyTo( srcImgtmp );
    cv::imshow( WINDOW_NAME, srcImg );
    //【1】准备参数
    g_rectangle = Rect(-1,-1,0,0);
    setMouseCallback(WINDOW_NAME,on_MouseHandle,(void*)&srcImg);

    while( 1 ){
        waitKey(10);
        if( contFlag ){
            cout << "( " << g_rectangle.x <<","<<g_rectangle.y<<" )" << g_rectangle.width << "x" << g_rectangle.height << endl;
            break;
        }
        srcImg.copyTo( srcImgtmp );
        if( g_bDrawingBox ) DrawRectangle( srcImgtmp, g_rectangle );
        imshow( WINDOW_NAME, srcImgtmp );
        continue;
    }

    while(1){
        imgPath = get_img_path( direcPath, imgNum, 4, "jpg" );
        srcImg = cv::imread( imgPath, IMREAD_COLOR );
        imgNum++;
        if( srcImg.empty() ){
            cout << "img empty" << endl;
            break;
        }
        cv::imshow( WINDOW_NAME, srcImg );
        if( waitKey( 30 ) == 27 ) break;//按下ESC键，程序退出
    }
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
    cv::rectangle(img,box.tl(),box.br(),cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255)));//随机颜色
}