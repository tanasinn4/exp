#define PI 3.14159265358979 //円周率
#define PART 100 //分割数
#include <stdio.h>
#include <windows.h> 
#include <mmsystem.h> 
#include <math.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <conio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

////////////////////////////////////////
// Global variables definition
////////////////////////////////////////
bool answerFlag = true;	//回答入力の有無(入力済み==true)
bool setFlag = false;	//次セット開始入力の有無(入力済み==true)
bool flag_stereo        = true;
bool flag_fullscreen    = true;
bool flag_hidecursor    = true;
bool flag_antialiasing  = true;

static double init_screen_size_x=0;	//ウィンドウサイズ。後で設定ファイル読み込み
static double init_screen_size_y=0;
static double CLR=0;	//円の輝度
static double CLG=0;
static double CLB=0;
static double BGLR=0;	//背景の輝度
static double BGLG=0;
static double BGLB=0;
static double LC=0;

static int CRPOS=0;	//conditioning円位置

const double init_screen_pos_x  = 0;
const double init_screen_pos_y  = 0;
const DWORD T0=0;	//起動から開始までのアイドル時間(ms)
const DWORD AT=250; //condition円点灯（消灯）時間(ms)
const DWORD BT=300;	//test,reference円表示期間(ms)
const DWORD ANS=2000;	//回答用最短待ち時間(ms)
const DWORD WAIT=1000;	//回答後次発刺激提示までのインターバル(ms)
const int BLINK=4;	//conditioning円点滅回数
//const GLclampf BGC=0.0;	//背景色(0:黒,0.5:グレー,1:白,黒白と等コントラストになるグレー:0.738)
//const GLclampf LC=1.0;	//ライン(クロスバー)色
//const GLclampf IC=1.0; //0.738;	//Inspection円色
//const GLclampf TRC=1.0; //0.738;	//test,reference円色
const double CB=0.01;	//クロスバー長さ
const double DIST=0.206;	//各円の注視点からの距離(3cmになるように)
const double TESTREFR=0.0687;	//test,reference円半径の基準(2cmになるように)
const double CONDRL=TESTREFR*1.5;	//0.075+0.005*4 大conditioning円半径(3cmになるように)
const double CONDRS=TESTREFR/2;	//0.075-0.005*8 小conditioning円半径(1cmになるように)
const double DEF=TESTREFR*0.025;	//relativesize変化量(85％～115%まで変化するように)
const int PRESRELA=1;	//各RELASIZEごとの表示回数
const int TRIAL=13*PRESRELA*4;	//トライアル(提示＆回答)回数＝RelativesizeごとにPRESRELA回ずつ×4種（大小中無）
const int WORKS=5;	//1セット中のワーク数
const int SETS=6;	//予定セット数

static DWORD start;	//開始時刻
static DWORD now,elapsed;	//経過時間
static DWORD last=T0-WAIT;	//前回トライアル終了時刻
static DWORD pressed=T0-WAIT;	//前回入力完了時刻
static int workscounter=0;	//実行済みワーク数（WORKSになったら終了）
static int setscounter=0;	//実行済みセット数（SETSになったら終了）

////////////////////////////////////////
// GLUT callback functions
////////////////////////////////////////
int GetRandom(int min,int max){	//乱数をmin～maxの範囲内の整数で出す
	return min + (int)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}
void crossbar(void){	//クロスバー表示関数
	glClearColor(BGLR, BGLG, BGLB, 1.0);	//背景色
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3d(LC, LC, LC);	//ライン色
	glBegin(GL_LINE_LOOP);	//横棒
	glVertex2d(-CB*init_screen_size_y/init_screen_size_x, 0.0);
	glVertex2d(CB*init_screen_size_y/init_screen_size_x, 0.0);
	glEnd();
	glBegin(GL_LINE_LOOP);	//縦棒
	glVertex2d(0.0, -CB);
	glVertex2d(0.0, CB);	
	glEnd();		
}
void circle(double a,double b,double r){	//円表示関数、引数は中心座標と半径
	glBegin(GL_POLYGON);	
	double x, y;	//描画用座標
	int i, n = PART;
	double rate;
	for (i = 0; i < n; i++) {	//座標を計算
		rate = (double)i / n;
		x = r * cos(2.0 * PI * rate);	
		y = r * sin(2.0 * PI * rate);
		glVertex2f((x-a)*init_screen_size_y/init_screen_size_x, y-b); 
	}
	glEnd();
}
void rest(void){
	void *font = GLUT_BITMAP_TIMES_ROMAN_24;//GLUT_BITMAP_HELVETICA_18;
	double charaposx=0.0;
	double charaposy=0.3;
	char str1[]="0";	//画面表示用セット数格納
	char str2[]="0";	//画面表示用セット数格納
	
	glClearColor(BGLR, BGLG, BGLB, 1.0);	//背景色
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3d(LC, LC, LC);	//ライン色
	if(setscounter==0&&workscounter==0){	//初回："Start"
		glRasterPos3f(charaposx-0.03*2,charaposy,0);
		glutBitmapCharacter(font, 'S');
		glRasterPos3f(charaposx-0.03*1,charaposy,0);
		glutBitmapCharacter(font, 't');
		glRasterPos3f(charaposx-0.03*0,charaposy,0);
		glutBitmapCharacter(font, 'a');
		glRasterPos3f(charaposx+0.03*1,charaposy,0);
		glutBitmapCharacter(font, 'r');
		glRasterPos3f(charaposx+0.03*2,charaposy,0);
		glutBitmapCharacter(font, 't');
	}
	else{
		if(workscounter==0){	//5workごとに"Rest"
			glRasterPos3f(charaposx-0.03*2,charaposy,0);
			glutBitmapCharacter(font, 'R');
			glRasterPos3f(charaposx-0.03*1,charaposy,0);
			glutBitmapCharacter(font, 'e');
			glRasterPos3f(charaposx-0.03*0,charaposy,0);
			glutBitmapCharacter(font, 's');
			glRasterPos3f(charaposx+0.03*1,charaposy,0);
			glutBitmapCharacter(font, 't');
		}
		else{	//それ以外は進捗表示
			sprintf(str1, "%d", setscounter+1);	
			sprintf(str2, "%d", workscounter);	

			glRasterPos3f(charaposx-0.03*6,charaposy,0);
			glutBitmapCharacter(font, 'E');
			glRasterPos3f(charaposx-0.03*5,charaposy,0);
			glutBitmapCharacter(font, 'n');
			glRasterPos3f(charaposx-0.03*4,charaposy,0);
			glutBitmapCharacter(font, 'd');
			glRasterPos3f(charaposx-0.03*3,charaposy,0);
			glutBitmapCharacter(font, ' ');
			glRasterPos3f(charaposx-0.03*2,charaposy,0);
			glutBitmapCharacter(font, 'o');
			glRasterPos3f(charaposx-0.03*1,charaposy,0);
			glutBitmapCharacter(font, 'f');
			glRasterPos3f(charaposx-0.03*0,charaposy,0);
			glutBitmapCharacter(font, ' ');
			glRasterPos3f(charaposx+0.03*1,charaposy,0);
			glutBitmapCharacter(font, *str1);
			glRasterPos3f(charaposx+0.03*2,charaposy,0);
			glutBitmapCharacter(font, '-');
			glRasterPos3f(charaposx+0.03*3,charaposy,0);
			glutBitmapCharacter(font, *str2);
			glRasterPos3f(charaposx+0.03*4,charaposy,0);
			glutBitmapCharacter(font, ' ');
			glRasterPos3f(charaposx+0.03*5,charaposy,0);
			glutBitmapCharacter(font, 'S');
			glRasterPos3f(charaposx+0.03*6,charaposy,0);
			glutBitmapCharacter(font, 'e');
			glRasterPos3f(charaposx+0.03*7,charaposy,0);
			glutBitmapCharacter(font, 't');
		}
	}
	glBegin(GL_LINE_LOOP);	//横棒
	glVertex2d(-CB*init_screen_size_y/init_screen_size_x, 0.0);
	glVertex2d(CB*init_screen_size_y/init_screen_size_x, 0.0);
	glEnd();
	glBegin(GL_LINE_LOOP);	//縦棒
	glVertex2d(0.0, -CB);
	glVertex2d(0.0, CB);	
	glEnd();		
}
void Idle(void){
	glutPostRedisplay();
}
//void Reshape(int w, int h){}
//void Visibility(int state){}
void Keyboard(unsigned char c, int x, int y){
	if(c==0x1B){	//ESCキーで終了
		exit(0);
	}
	else{	//SPACEキーで刺激提示開始
		setFlag = true;
	}
}
void KeyboardSp(int c, int x, int y){	//回答受付用関数
	int n=9;	//1,0以外の初期値として。
	if(answerFlag == false){	//既に回答していたらスルー、未回答なら書き出しへ
		ofstream ofs("data000.txt",std::ios::out|std::ios::app|std::ios::ate);
		if(c==GLUT_KEY_LEFT){	//[←]キーを押した場合
			if(CRPOS==1){	//左がtestならtestが大だから1を記録
				n=1;
			}
			else if(CRPOS==2){	//右がtestならref大だから0を記録
				n=0;
			}

		}
		else if(c==GLUT_KEY_RIGHT){	//[→]キーを押した場合
			if(CRPOS==1){	//左がtestならref大だから0を記録
				n=0;
			}
			else if(CRPOS==2){	//右がtestならtestが大だから1を記録
				n=1;
			}
		}
		ofs<<n<<endl;	// 被験者回答書き出し(無効なキーによる回答はn=9)
		now = timeGetTime();	
		pressed = (now - start);	//入力完了時刻計算
		answerFlag = true;	//回答完了フラグたてる
	}
}
//void Mouse(int button, int state, int x, int y){}
//void MouseMotion(int x, int y){}
//void MouseMotionPassive(int x, int y){}
//void MouseEntry(int state){}
void test(int CRPOS,int RELASIZE) {	//condの位置＆relativesizeを引数にtest,reference円表示
	double r1,r2;	//test円半径,ref	erence円半径
	double a=DIST; //注視点からの距離
	double b=0.0; //注視点からのy距離

	crossbar();	//注視点クロスバー表示				 
	glColor3d(CLR, CLG, CLB);	//test,reference円の色
	switch (RELASIZE) {
		case 1:
			r1=TESTREFR-8*DEF;
			break;
		case 2:
			r1=TESTREFR-6*DEF;
			break;
        case 3:
			r1=TESTREFR-4*DEF;
			break;
        case 4:
			r1=TESTREFR-3*DEF;
			break;
		case 5:
			r1=TESTREFR-2*DEF;
			break;
		case 6:
			r1=TESTREFR-1*DEF;
			break;
		case 7:
			r1=TESTREFR;
			break;
		case 8:
			r1=TESTREFR+1*DEF;
			break;
		case 9:
			r1=TESTREFR+2*DEF;
			break;
		case 10:
			r1=TESTREFR+3*DEF;
			break;
		case 11:
			r1=TESTREFR+4*DEF;
			break;
		case 12:
			r1=TESTREFR+6*DEF;
			break;
		case 13:
			r1=TESTREFR+8*DEF;
			break;
    }
	r2=TESTREFR;
	switch (CRPOS) {
		case 1:	//左condなら左がtest
			circle(a,b,r1);// test円を描画
			circle(-a,b,r2);// reference円を描画
			break;
		case 2:	//右condなら右がtest
			circle(a,b,r2);// reference円を描画
			circle(-a,b,r1);// test円を描画
			break;
	}
	glutSwapBuffers();	
	Sleep(BT);//test,reference円表示時間BT
}
void Display(void){		//刺激提示
	double r=0;	//半径
	double a=DIST; //注視点からのx距離
	double b=0.0; //注視点からのy距離
	
	int fc=0;	//点滅回数カウント用
	int tcounter=13;	//表示回数確認用 最初にループ入らせる(1回目のCRSIZEランダム取得の)ために13で初期化
	int CRSIZE,RELASIZE;	//提示刺激情報
	int arrayn=0;
	static int it=1;	//実行トライアル番号
	static int il=0;	//大cond表示済み回数
	static int is=0;	//小cond表示済み回数
	static int im=0;	//中cond表示済み回数
	static int in=0;	//condなし表示済み回数
	static int presentlarge[13]={0};	//各relativesizeについての大きいconditioning円表示回数	
	static int presentsmall[13]={0};	//各relativesizeについての小さいconditioning円表示回数
	static int presentmiddle[13]={0};	//各relativesizeについての中間のconditioning円表示回数
	static int presentnone[13]={0};	//各relativesizeについてのconditioning円なし表示回数
	
	time_t timer;
	time(&timer);	//記録用の現在時刻取得 
 	ofstream ofs("data000.txt",std::ios::app |std::ios::ate);
 
	now = timeGetTime();	//現在時刻取得
	elapsed = (now - start);	//経過時間計算

	if(setFlag==false){	//次セット開始キー未入力なら
		rest();	//休憩画面提示
		glutSwapBuffers();
	}
	if(setFlag==true){	//次セット開始キー入力済みなら
		if(it <= TRIAL) {	//「提示→回答」がTRIAL回になるまで繰り返す
			if(elapsed > pressed+WAIT&&answerFlag){	//前回の回答終了からWAIT秒経過（回答受付期間外）かつ被験者キー入力済みなら刺激提示
				if(setscounter==0&&workscounter==0&&it==1){	//第一セットの第一トライアルのみ、実験開始日時を書き出し	
					ofs << "*****EXPERIMENT on " << ctime(&timer) <<flush;// 時刻書き出し
					ofs << "position,condsize,relativesize,answer"<<endl;	//ヘッダー
					srand((unsigned int)time(NULL));//毎回乱数を変えるための初期化
				}
				CRPOS = GetRandom(1,2);	//conditioning円の左or右…どちらでもよい（完全ランダム）									
				while(tcounter==13*PRESRELA){	//得たCRSIZEでの表示が規定回数終わっていれば、まだ残っているCRSIZEが出るまで再取得
					CRSIZE = GetRandom(1,4);	//conditioning円の大or小or中orなし
					switch (CRSIZE){
						case 1:
							tcounter=il;
							break;
						case 2:
							tcounter=is;
							break;
						case 3:
							tcounter=im;
							break;
						case 4:
							tcounter=in;
							break;				
					}
				}			
				RELASIZE = GetRandom(1,13);//乱数生成：Relative size(test/reference)13段階
				if(CRSIZE==1){	//conditioning円サイズ大と乱数が出たら
					r = CONDRL;	//大
					il++;	//cond大の表示回数を+1
					while(presentlarge[RELASIZE-1]==PRESRELA){	//大cond円でそのRELASIZEが既にPRESRELA回表示していたら
						RELASIZE = GetRandom(1,13);	//それ以外のRELASIZEが出るまで乱数取得リトライ
					}
					presentlarge[RELASIZE-1]++;	//そのRELASIZEの表示回数を１回カウントする
				}
				else if(CRSIZE==2){	//conditioning円サイズ小
					r = CONDRS;
					is++;
					while(presentsmall[RELASIZE-1]==PRESRELA){	
						RELASIZE = GetRandom(1,13);	
					}
					presentsmall[RELASIZE-1]++;
				}
				else if(CRSIZE==3){	//conditioning円サイズ中
					r = TESTREFR;
					im++;
					while(presentmiddle[RELASIZE-1]==PRESRELA){	
						RELASIZE = GetRandom(1,13);
					}
					presentmiddle[RELASIZE-1]++;
				}
				else if(CRSIZE==4){	//conditioning円なし
					r = 0;	//なし
					in++;
					while(presentnone[RELASIZE-1]==PRESRELA){
						RELASIZE = GetRandom(1,13);
					}
					presentnone[RELASIZE-1]++;
				}
				
				ofs << CRPOS <<","<<CRSIZE<<','<<RELASIZE <<","<<flush;// 提示情報確定後、cond円情報とrelativesize書き出し
								
				while(fc < BLINK){	//conditioning期間（n回点滅)
					crossbar();	//クロスバー表示
					glColor3d(CLR, CLG, CLB);	//Inspection円の色
					switch (CRPOS) {	// conditioning円位置確認後描画
						case 1:
							circle(a,b,r);
							break;
						case 2:
							circle(-a,b,r);
							break;
					}
					glutSwapBuffers();
					Sleep(AT);	//conditioning円表示時間
					crossbar();	//クロスバー表示
					glutSwapBuffers();
					Sleep(AT);	//conditioning円非表示時間
					fc++;	//点滅回数カウント
				}
				test(CRPOS,RELASIZE);	//test,reference表示
				crossbar();
				glutSwapBuffers();
				now = timeGetTime();	//現在時刻取得
				last = (now - start);	//刺激提示終了時間計算
				it++;	//１トライアルをカウント
				answerFlag = false;	//フラグを未回答状態に
			}		
			else if(elapsed<last+ANS&&answerFlag==false){	//回答受付期間内かつ入力前なら表示なし
				glClearColor(BGLR, BGLG, BGLB, 1.0);	//背景色
				glClear(GL_COLOR_BUFFER_BIT);
				glutSwapBuffers();
			}		
			else {		//回答受付期間内かつ、入力済みまたは定時後ANS秒経過後ならクロスバーのみ提示
				crossbar();
				glutSwapBuffers();
			}
		}
		else if(it>TRIAL){	//全トライアル完了していたら
			if(elapsed>last+ANS&&answerFlag&&setFlag==true){	//回答待ち時間後＆入力完了済み＆セット進行中なら
				workscounter++;
				it=1;//各カウンターリセット
				il=0;
				is=0;
				im=0;
				in=0;
				for(arrayn = 0; arrayn < 13; arrayn++){
					presentlarge[arrayn] = 0;
					presentsmall[arrayn] = 0;
					presentmiddle[arrayn] = 0;	
					presentnone[arrayn] = 0;
				}
				setFlag = false;	//次セット開始キーを未入力状態に。
				
				if(workscounter==WORKS){	//1セット内のワーク数を満了していたら
					setscounter++;	//1セット完了をカウント	
					if(setscounter==SETS){	//全セットすべて完了済みなら
						ofs << "*****EXPERIMENT finished "<<endl;// 終了マーカー書き込み
						exit(0);	//ウィンドウ閉じる
					}
					workscounter=0;	//ワーク数カウントをリセット
					ofs <<endl;// 終了改行書き込み
				}				
			}
			else{	//ラスト１回の入力時間を待つ
				crossbar();
				glutSwapBuffers();
			}
		}
	}
}
////////////////////////////////////////
// Other functions
////////////////////////////////////////
class CistreamCsv {
	// マニピュレータ endl の定義
	friend CistreamCsv& endl(CistreamCsv& riscsv) {
		// 行末まで読み飛ばす。
		int iChar = 0;
		while ((iChar = riscsv.m_ris.get()) != '\n' && iChar != EOF) {}
		return riscsv;
	}
private:
	istream& m_ris;		// 入力に使う istream の参照
public:
	CistreamCsv(istream& ris) : m_ris(ris) {}
	CistreamCsv& operator >>(int& ri) {
		ri = 0;
		int iChar = 0;
		// 空白を読み飛ばす。
		while (isspace(iChar = m_ris.get()) && iChar != '\n') {}
		if (iChar == EOF ) { return *this; }
		string strValue;
		if (iChar == '\n') { m_ris.putback(iChar); iChar = 0; }
		else			   { strValue += iChar; 			  }
		// コンマまで読む。
		while ((iChar = m_ris.get()) != ',' && iChar != EOF && iChar != '\n') {
			strValue += iChar;
		}
		ri = atoi(strValue.c_str());
		if (iChar == EOF ) { return *this;		   }
		if (iChar == '\n') { m_ris.putback(iChar); }
		return *this;
	}
	// マニピュレータを受け入れるための挿入演算子
	CistreamCsv& operator >>(CistreamCsv& (*pmanipulatorfunction)(CistreamCsv&)) {
		return (*pmanipulatorfunction)(*this);
	}
};
void getvalue(){
	int i,aiTable[9];
	
	string name;	//名前入力用
	ifstream ifs("setting.csv");	// データファイル開く

	getline(ifs, name);
	string token;
	istringstream stream(name); //文字列を読み込み
	getline( stream, token, ',' );

	CistreamCsv istreamcsv(ifs);
	for (i=0; i<9; ++i){                 
		istreamcsv >> aiTable[i];	// 要素を一つ読み込む。コンマは自動処理される。
		switch (i){
			case 0:
				init_screen_size_x=aiTable[i];   // 要素を一つ読み込んで次の行に移る。
				break;
			case 1:
				init_screen_size_y=aiTable[i]; 
				break;		
			case 2:
				CLR=aiTable[i];
				CLR=CLR/1000;
				break;		
			case 3:
				CLG=aiTable[i];
				CLG=CLG/1000;
				break;		
			case 4:
				CLB=aiTable[i]; 
				CLB=CLB/1000;
				break;		
			case 5:
				BGLR=aiTable[i]; 
				BGLR=BGLR/1000;
				break;		
			case 6:
				BGLG=aiTable[i]; 
				BGLG=BGLG/1000;
				break;		
			case 7:
				BGLB=aiTable[i]; 
				BGLB=BGLB/1000;
				break;		
			case 8:
				LC=aiTable[i]; 
				LC=LC/1000;
				break;		
		}
		istreamcsv >> endl;	// 次の行に移る。
	}
}
void Init(void){	//スタート処理
	static DWORD start = timeGetTime();	//開始時刻取得
	getvalue();	//外部設定ファイル読み込み
}
////////////////////////////////////////
// main
////////////////////////////////////////
int main(int argc, char *argv[]){
    Init();
    glutInit(&argc, argv);
    if (flag_stereo)
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_STEREO );
    else
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
    glutInitWindowSize(init_screen_size_x, init_screen_size_y);
    glutInitWindowPosition(init_screen_pos_x, init_screen_pos_y);
    glutCreateWindow(argv[0]);

    if (flag_fullscreen)	//フルスクリーン
        glutFullScreen();
    if (flag_hidecursor)	//カーソル非表示
        glutSetCursor(GLUT_CURSOR_NONE);
    if (flag_antialiasing) {	//アンチエイリアス
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_DEPTH_TEST);
    }
	glutDisplayFunc(Display);	//提示
    glutIdleFunc(Idle);	//再描画
//    glutReshapeFunc(Reshape);
//    glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);	//開始or強制終了キー入力
    glutSpecialFunc(KeyboardSp);	//回答受付
//    glutMouseFunc(Mouse);
//    glutMotionFunc(MouseMotion);
//    glutPassiveMotionFunc(MouseMotionPassive);
//    glutEntryFunc(MouseEntry);
    glutMainLoop(); 
    return 0;
}