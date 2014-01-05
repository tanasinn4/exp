#define PI 3.14159265358979 //�~����
#define PART 100 //������
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
bool answerFlag = true;	//�񓚓��̗͂L��(���͍ς�==true)
bool setFlag = false;	//���Z�b�g�J�n���̗͂L��(���͍ς�==true)
bool flag_stereo        = true;
bool flag_fullscreen    = true;
bool flag_hidecursor    = true;
bool flag_antialiasing  = true;

static double init_screen_size_x=0;	//�E�B���h�E�T�C�Y�B��Őݒ�t�@�C���ǂݍ���
static double init_screen_size_y=0;
static double CLR=0;	//�~�̋P�x
static double CLG=0;
static double CLB=0;
static double BGLR=0;	//�w�i�̋P�x
static double BGLG=0;
static double BGLB=0;
static double LC=0;

static int CRPOS=0;	//conditioning�~�ʒu

const double init_screen_pos_x  = 0;
const double init_screen_pos_y  = 0;
const DWORD T0=0;	//�N������J�n�܂ł̃A�C�h������(ms)
const DWORD AT=250; //condition�~�_���i�����j����(ms)
const DWORD BT=300;	//test,reference�~�\������(ms)
const DWORD ANS=2000;	//�񓚗p�ŒZ�҂�����(ms)
const DWORD WAIT=1000;	//�񓚌㎟���h���񎦂܂ł̃C���^�[�o��(ms)
const int BLINK=4;	//conditioning�~�_�ŉ�
//const GLclampf BGC=0.0;	//�w�i�F(0:��,0.5:�O���[,1:��,�����Ɠ��R���g���X�g�ɂȂ�O���[:0.738)
//const GLclampf LC=1.0;	//���C��(�N���X�o�[)�F
//const GLclampf IC=1.0; //0.738;	//Inspection�~�F
//const GLclampf TRC=1.0; //0.738;	//test,reference�~�F
const double CB=0.01;	//�N���X�o�[����
const double DIST=0.206;	//�e�~�̒����_����̋���(3cm�ɂȂ�悤��)
const double TESTREFR=0.0687;	//test,reference�~���a�̊(2cm�ɂȂ�悤��)
const double CONDRL=TESTREFR*1.5;	//0.075+0.005*4 ��conditioning�~���a(3cm�ɂȂ�悤��)
const double CONDRS=TESTREFR/2;	//0.075-0.005*8 ��conditioning�~���a(1cm�ɂȂ�悤��)
const double DEF=TESTREFR*0.025;	//relativesize�ω���(85���`115%�܂ŕω�����悤��)
const int PRESRELA=1;	//�eRELASIZE���Ƃ̕\����
const int TRIAL=13*PRESRELA*4;	//�g���C�A��(�񎦁���)�񐔁�Relativesize���Ƃ�PRESRELA�񂸂~4��i�召�����j
const int WORKS=5;	//1�Z�b�g���̃��[�N��
const int SETS=6;	//�\��Z�b�g��

static DWORD start;	//�J�n����
static DWORD now,elapsed;	//�o�ߎ���
static DWORD last=T0-WAIT;	//�O��g���C�A���I������
static DWORD pressed=T0-WAIT;	//�O����͊�������
static int workscounter=0;	//���s�ς݃��[�N���iWORKS�ɂȂ�����I���j
static int setscounter=0;	//���s�ς݃Z�b�g���iSETS�ɂȂ�����I���j

////////////////////////////////////////
// GLUT callback functions
////////////////////////////////////////
int GetRandom(int min,int max){	//������min�`max�͈͓̔��̐����ŏo��
	return min + (int)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}
void crossbar(void){	//�N���X�o�[�\���֐�
	glClearColor(BGLR, BGLG, BGLB, 1.0);	//�w�i�F
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3d(LC, LC, LC);	//���C���F
	glBegin(GL_LINE_LOOP);	//���_
	glVertex2d(-CB*init_screen_size_y/init_screen_size_x, 0.0);
	glVertex2d(CB*init_screen_size_y/init_screen_size_x, 0.0);
	glEnd();
	glBegin(GL_LINE_LOOP);	//�c�_
	glVertex2d(0.0, -CB);
	glVertex2d(0.0, CB);	
	glEnd();		
}
void circle(double a,double b,double r){	//�~�\���֐��A�����͒��S���W�Ɣ��a
	glBegin(GL_POLYGON);	
	double x, y;	//�`��p���W
	int i, n = PART;
	double rate;
	for (i = 0; i < n; i++) {	//���W���v�Z
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
	char str1[]="0";	//��ʕ\���p�Z�b�g���i�[
	char str2[]="0";	//��ʕ\���p�Z�b�g���i�[
	
	glClearColor(BGLR, BGLG, BGLB, 1.0);	//�w�i�F
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3d(LC, LC, LC);	//���C���F
	if(setscounter==0&&workscounter==0){	//����F"Start"
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
		if(workscounter==0){	//5work���Ƃ�"Rest"
			glRasterPos3f(charaposx-0.03*2,charaposy,0);
			glutBitmapCharacter(font, 'R');
			glRasterPos3f(charaposx-0.03*1,charaposy,0);
			glutBitmapCharacter(font, 'e');
			glRasterPos3f(charaposx-0.03*0,charaposy,0);
			glutBitmapCharacter(font, 's');
			glRasterPos3f(charaposx+0.03*1,charaposy,0);
			glutBitmapCharacter(font, 't');
		}
		else{	//����ȊO�͐i���\��
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
	glBegin(GL_LINE_LOOP);	//���_
	glVertex2d(-CB*init_screen_size_y/init_screen_size_x, 0.0);
	glVertex2d(CB*init_screen_size_y/init_screen_size_x, 0.0);
	glEnd();
	glBegin(GL_LINE_LOOP);	//�c�_
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
	if(c==0x1B){	//ESC�L�[�ŏI��
		exit(0);
	}
	else{	//SPACE�L�[�Ŏh���񎦊J�n
		setFlag = true;
	}
}
void KeyboardSp(int c, int x, int y){	//�񓚎�t�p�֐�
	int n=9;	//1,0�ȊO�̏����l�Ƃ��āB
	if(answerFlag == false){	//���ɉ񓚂��Ă�����X���[�A���񓚂Ȃ珑���o����
		ofstream ofs("data000.txt",std::ios::out|std::ios::app|std::ios::ate);
		if(c==GLUT_KEY_LEFT){	//[��]�L�[���������ꍇ
			if(CRPOS==1){	//����test�Ȃ�test���傾����1���L�^
				n=1;
			}
			else if(CRPOS==2){	//�E��test�Ȃ�ref�傾����0���L�^
				n=0;
			}

		}
		else if(c==GLUT_KEY_RIGHT){	//[��]�L�[���������ꍇ
			if(CRPOS==1){	//����test�Ȃ�ref�傾����0���L�^
				n=0;
			}
			else if(CRPOS==2){	//�E��test�Ȃ�test���傾����1���L�^
				n=1;
			}
		}
		ofs<<n<<endl;	// �팱�҉񓚏����o��(�����ȃL�[�ɂ��񓚂�n=9)
		now = timeGetTime();	
		pressed = (now - start);	//���͊��������v�Z
		answerFlag = true;	//�񓚊����t���O���Ă�
	}
}
//void Mouse(int button, int state, int x, int y){}
//void MouseMotion(int x, int y){}
//void MouseMotionPassive(int x, int y){}
//void MouseEntry(int state){}
void test(int CRPOS,int RELASIZE) {	//cond�̈ʒu��relativesize��������test,reference�~�\��
	double r1,r2;	//test�~���a,ref	erence�~���a
	double a=DIST; //�����_����̋���
	double b=0.0; //�����_�����y����

	crossbar();	//�����_�N���X�o�[�\��				 
	glColor3d(CLR, CLG, CLB);	//test,reference�~�̐F
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
		case 1:	//��cond�Ȃ獶��test
			circle(a,b,r1);// test�~��`��
			circle(-a,b,r2);// reference�~��`��
			break;
		case 2:	//�Econd�Ȃ�E��test
			circle(a,b,r2);// reference�~��`��
			circle(-a,b,r1);// test�~��`��
			break;
	}
	glutSwapBuffers();	
	Sleep(BT);//test,reference�~�\������BT
}
void Display(void){		//�h����
	double r=0;	//���a
	double a=DIST; //�����_�����x����
	double b=0.0; //�����_�����y����
	
	int fc=0;	//�_�ŉ񐔃J�E���g�p
	int tcounter=13;	//�\���񐔊m�F�p �ŏ��Ƀ��[�v���点��(1��ڂ�CRSIZE�����_���擾��)���߂�13�ŏ�����
	int CRSIZE,RELASIZE;	//�񎦎h�����
	int arrayn=0;
	static int it=1;	//���s�g���C�A���ԍ�
	static int il=0;	//��cond�\���ς݉�
	static int is=0;	//��cond�\���ς݉�
	static int im=0;	//��cond�\���ς݉�
	static int in=0;	//cond�Ȃ��\���ς݉�
	static int presentlarge[13]={0};	//�erelativesize�ɂ��Ă̑傫��conditioning�~�\����	
	static int presentsmall[13]={0};	//�erelativesize�ɂ��Ă̏�����conditioning�~�\����
	static int presentmiddle[13]={0};	//�erelativesize�ɂ��Ă̒��Ԃ�conditioning�~�\����
	static int presentnone[13]={0};	//�erelativesize�ɂ��Ă�conditioning�~�Ȃ��\����
	
	time_t timer;
	time(&timer);	//�L�^�p�̌��ݎ����擾 
 	ofstream ofs("data000.txt",std::ios::app |std::ios::ate);
 
	now = timeGetTime();	//���ݎ����擾
	elapsed = (now - start);	//�o�ߎ��Ԍv�Z

	if(setFlag==false){	//���Z�b�g�J�n�L�[�����͂Ȃ�
		rest();	//�x�e��ʒ�
		glutSwapBuffers();
	}
	if(setFlag==true){	//���Z�b�g�J�n�L�[���͍ς݂Ȃ�
		if(it <= TRIAL) {	//�u�񎦁��񓚁v��TRIAL��ɂȂ�܂ŌJ��Ԃ�
			if(elapsed > pressed+WAIT&&answerFlag){	//�O��̉񓚏I������WAIT�b�o�߁i�񓚎�t���ԊO�j���팱�҃L�[���͍ς݂Ȃ�h����
				if(setscounter==0&&workscounter==0&&it==1){	//���Z�b�g�̑��g���C�A���̂݁A�����J�n�����������o��	
					ofs << "*****EXPERIMENT on " << ctime(&timer) <<flush;// ���������o��
					ofs << "position,condsize,relativesize,answer"<<endl;	//�w�b�_�[
					srand((unsigned int)time(NULL));//���񗐐���ς��邽�߂̏�����
				}
				CRPOS = GetRandom(1,2);	//conditioning�~�̍�or�E�c�ǂ���ł��悢�i���S�����_���j									
				while(tcounter==13*PRESRELA){	//����CRSIZE�ł̕\�����K��񐔏I����Ă���΁A�܂��c���Ă���CRSIZE���o��܂ōĎ擾
					CRSIZE = GetRandom(1,4);	//conditioning�~�̑�or��or��or�Ȃ�
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
				RELASIZE = GetRandom(1,13);//���������FRelative size(test/reference)13�i�K
				if(CRSIZE==1){	//conditioning�~�T�C�Y��Ɨ������o����
					r = CONDRL;	//��
					il++;	//cond��̕\���񐔂�+1
					while(presentlarge[RELASIZE-1]==PRESRELA){	//��cond�~�ł���RELASIZE������PRESRELA��\�����Ă�����
						RELASIZE = GetRandom(1,13);	//����ȊO��RELASIZE���o��܂ŗ����擾���g���C
					}
					presentlarge[RELASIZE-1]++;	//����RELASIZE�̕\���񐔂��P��J�E���g����
				}
				else if(CRSIZE==2){	//conditioning�~�T�C�Y��
					r = CONDRS;
					is++;
					while(presentsmall[RELASIZE-1]==PRESRELA){	
						RELASIZE = GetRandom(1,13);	
					}
					presentsmall[RELASIZE-1]++;
				}
				else if(CRSIZE==3){	//conditioning�~�T�C�Y��
					r = TESTREFR;
					im++;
					while(presentmiddle[RELASIZE-1]==PRESRELA){	
						RELASIZE = GetRandom(1,13);
					}
					presentmiddle[RELASIZE-1]++;
				}
				else if(CRSIZE==4){	//conditioning�~�Ȃ�
					r = 0;	//�Ȃ�
					in++;
					while(presentnone[RELASIZE-1]==PRESRELA){
						RELASIZE = GetRandom(1,13);
					}
					presentnone[RELASIZE-1]++;
				}
				
				ofs << CRPOS <<","<<CRSIZE<<','<<RELASIZE <<","<<flush;// �񎦏��m���Acond�~����relativesize�����o��
								
				while(fc < BLINK){	//conditioning���ԁin��_��)
					crossbar();	//�N���X�o�[�\��
					glColor3d(CLR, CLG, CLB);	//Inspection�~�̐F
					switch (CRPOS) {	// conditioning�~�ʒu�m�F��`��
						case 1:
							circle(a,b,r);
							break;
						case 2:
							circle(-a,b,r);
							break;
					}
					glutSwapBuffers();
					Sleep(AT);	//conditioning�~�\������
					crossbar();	//�N���X�o�[�\��
					glutSwapBuffers();
					Sleep(AT);	//conditioning�~��\������
					fc++;	//�_�ŉ񐔃J�E���g
				}
				test(CRPOS,RELASIZE);	//test,reference�\��
				crossbar();
				glutSwapBuffers();
				now = timeGetTime();	//���ݎ����擾
				last = (now - start);	//�h���񎦏I�����Ԍv�Z
				it++;	//�P�g���C�A�����J�E���g
				answerFlag = false;	//�t���O�𖢉񓚏�Ԃ�
			}		
			else if(elapsed<last+ANS&&answerFlag==false){	//�񓚎�t���ԓ������͑O�Ȃ�\���Ȃ�
				glClearColor(BGLR, BGLG, BGLB, 1.0);	//�w�i�F
				glClear(GL_COLOR_BUFFER_BIT);
				glutSwapBuffers();
			}		
			else {		//�񓚎�t���ԓ����A���͍ς݂܂��͒莞��ANS�b�o�ߌ�Ȃ�N���X�o�[�̂ݒ�
				crossbar();
				glutSwapBuffers();
			}
		}
		else if(it>TRIAL){	//�S�g���C�A���������Ă�����
			if(elapsed>last+ANS&&answerFlag&&setFlag==true){	//�񓚑҂����Ԍさ���͊����ς݁��Z�b�g�i�s���Ȃ�
				workscounter++;
				it=1;//�e�J�E���^�[���Z�b�g
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
				setFlag = false;	//���Z�b�g�J�n�L�[�𖢓��͏�ԂɁB
				
				if(workscounter==WORKS){	//1�Z�b�g���̃��[�N���𖞗����Ă�����
					setscounter++;	//1�Z�b�g�������J�E���g	
					if(setscounter==SETS){	//�S�Z�b�g���ׂĊ����ς݂Ȃ�
						ofs << "*****EXPERIMENT finished "<<endl;// �I���}�[�J�[��������
						exit(0);	//�E�B���h�E����
					}
					workscounter=0;	//���[�N���J�E���g�����Z�b�g
					ofs <<endl;// �I�����s��������
				}				
			}
			else{	//���X�g�P��̓��͎��Ԃ�҂�
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
	// �}�j�s�����[�^ endl �̒�`
	friend CistreamCsv& endl(CistreamCsv& riscsv) {
		// �s���܂œǂݔ�΂��B
		int iChar = 0;
		while ((iChar = riscsv.m_ris.get()) != '\n' && iChar != EOF) {}
		return riscsv;
	}
private:
	istream& m_ris;		// ���͂Ɏg�� istream �̎Q��
public:
	CistreamCsv(istream& ris) : m_ris(ris) {}
	CistreamCsv& operator >>(int& ri) {
		ri = 0;
		int iChar = 0;
		// �󔒂�ǂݔ�΂��B
		while (isspace(iChar = m_ris.get()) && iChar != '\n') {}
		if (iChar == EOF ) { return *this; }
		string strValue;
		if (iChar == '\n') { m_ris.putback(iChar); iChar = 0; }
		else			   { strValue += iChar; 			  }
		// �R���}�܂œǂށB
		while ((iChar = m_ris.get()) != ',' && iChar != EOF && iChar != '\n') {
			strValue += iChar;
		}
		ri = atoi(strValue.c_str());
		if (iChar == EOF ) { return *this;		   }
		if (iChar == '\n') { m_ris.putback(iChar); }
		return *this;
	}
	// �}�j�s�����[�^���󂯓���邽�߂̑}�����Z�q
	CistreamCsv& operator >>(CistreamCsv& (*pmanipulatorfunction)(CistreamCsv&)) {
		return (*pmanipulatorfunction)(*this);
	}
};
void getvalue(){
	int i,aiTable[9];
	
	string name;	//���O���͗p
	ifstream ifs("setting.csv");	// �f�[�^�t�@�C���J��

	getline(ifs, name);
	string token;
	istringstream stream(name); //�������ǂݍ���
	getline( stream, token, ',' );

	CistreamCsv istreamcsv(ifs);
	for (i=0; i<9; ++i){                 
		istreamcsv >> aiTable[i];	// �v�f����ǂݍ��ށB�R���}�͎������������B
		switch (i){
			case 0:
				init_screen_size_x=aiTable[i];   // �v�f����ǂݍ���Ŏ��̍s�Ɉڂ�B
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
		istreamcsv >> endl;	// ���̍s�Ɉڂ�B
	}
}
void Init(void){	//�X�^�[�g����
	static DWORD start = timeGetTime();	//�J�n�����擾
	getvalue();	//�O���ݒ�t�@�C���ǂݍ���
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

    if (flag_fullscreen)	//�t���X�N���[��
        glutFullScreen();
    if (flag_hidecursor)	//�J�[�\����\��
        glutSetCursor(GLUT_CURSOR_NONE);
    if (flag_antialiasing) {	//�A���`�G�C���A�X
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_DEPTH_TEST);
    }
	glutDisplayFunc(Display);	//��
    glutIdleFunc(Idle);	//�ĕ`��
//    glutReshapeFunc(Reshape);
//    glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);	//�J�nor�����I���L�[����
    glutSpecialFunc(KeyboardSp);	//�񓚎�t
//    glutMouseFunc(Mouse);
//    glutMotionFunc(MouseMotion);
//    glutPassiveMotionFunc(MouseMotionPassive);
//    glutEntryFunc(MouseEntry);
    glutMainLoop(); 
    return 0;
}