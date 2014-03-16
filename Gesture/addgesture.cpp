#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cv.h>
#include <highgui.h>

using namespace std;

struct cons{
    int car;
    int cdr;
};  

int sqr (int x) {return x*x;}
int prev[2]={0};

#include "crosshair.cpp"
#include "recognize2.cpp"

cons writecenter( IplImage* img ) {
	cons gest; gest.car = 0; gest.cdr = 0;
	int x, num=0, i;
	uchar * ptr; char* pt;
	long int x_tot=0, y_tot=0;
	for( int y=0; y<img->height; y++ ) {
		ptr = (uchar*) (img->imageData + y * img->widthStep);
		for(x=0; x<(img->width); x++) {
			if (ptr[3*x]!=127 || ptr[3*x+1]!=127 || ptr[3*x+2]!=127) {
				x_tot+=x;
				y_tot+=y;
				num++;
			}
		}
	}
	
	if (num>20) {
		gest.car = x_tot/num ;
		gest.cdr = y_tot/num ;
		//cout<<gest.car<<"      "<<gest.cdr<<"\n\n";
	}
	return gest;
}

void writeimg (cons fin[], int N, IplImage* res){
	int x, y, i, j, ht, wd;
	uchar * ptr;
	float slope; 
	char* pt;
		
		ht=res->height; wd = res->width;
	for(j=0; j<N; j++){
		x = fin[j].car + ht/2 ; 
		y = fin[j].cdr + wd/2 ;
		if (x<0)x=0;if (y<0)y=0;if (x>=wd)x=wd-1; if(y>=ht)y=ht-1;
	
		pt=res->imageData + y*res->widthStep + x;
			*((uchar*)pt)=255;
			if (prev[0]!=0 || prev[1]!=0) {
				slope=((float)(x-prev[0]))/(y-prev[1]);
				for (i=min((int)(y)-prev[1],0);
							i<max((int)(y)-prev[1],0);
							i++)
				{
					ptr=(uchar*)(pt-(int)(i*(res->widthStep)+i*slope));
					*ptr=255;
				}
			}
			prev[0]=x; prev[1]=y;
	}
}
	

void differ( IplImage* imgmid, IplImage* imgnew) {
	int k, x;
	float total=0, num=1.0/(3*(imgmid->width)*(imgmid->height)), sqs=0;
	for( int y=0; y<imgmid->height; y++ ) {
		uchar* ptrmid = (uchar*) (
		imgmid->imageData + y * imgmid->widthStep
		);
		uchar* ptrnew = (uchar*) (
		imgnew->imageData + y * imgnew->widthStep
		);
		for(x=0; x<3*(imgmid->width); x++ ) {
			k=ptrnew[x]-ptrmid[x];
			if (k > 30) ptrmid[x]=255;
			else if (k < -30) ptrmid[x]=0;
			else ptrmid[x]=127;
		}
	}
}


int main( int argc, char* argv[] ) {
	
	FILE *fp;
	//cvNamedWindow( "Recording ...press ESC to stop !", CV_WINDOW_AUTOSIZE );
	bool a=false;
	char g='\0',l='\0', str1[8]="bg";
	char* cmmd;
	cons x, gest[32], fin[32];
	int i=0,N=32,e,j,q;
	if(argv[1]==NULL || argv[2]==NULL){
		cout<<"Invalid arguments\nUse as:\n";
		cout<<"addgesture [geture-character] [custom-command]\n\n";
		return -1;
		}
	cmmd = argv[1];
	g = cmmd[0]; i=0;
	
	while(str1[i]!='\0'){
		if (g == str1[i]){
			cout<<"Gesture for '"<<g<<"' exists and cannot be reset.\nPlease try again ...\n\n";
			return -1;
		}
		i++;
	}
	
	cmmd = argv[2];
	CvCapture* capture = cvCreateCameraCapture( 0 );

	IplImage *mid = cvQueryFrame(capture);
	IplImage *frames = cvQueryFrame(capture);
	
	IplImage* res = cvCreateImage(
		cvGetSize(frames),
		IPL_DEPTH_8U,
		1
	);
	
	for(e=0; e<32; e++){
		fin[e].car = 0;
		fin[e].cdr = 0;
	}
	
	for (e=0 ; e<1 ; e++){
		cout<<"Perform the gesture :\n";
		a = false; i=0;
		cvNamedWindow( "Recording ...press ESC to stop !", CV_WINDOW_AUTOSIZE );
		
		while (true) {
				
				mid = cvCloneImage(frames);
				frames = cvQueryFrame( capture );
				if (!frames) break;
				differ(mid,frames);
				crosshair(mid);
	
				cvShowImage( "Recording ...press ESC to stop !",mid );
				char c = cvWaitKey(33);
				if( c == 27 ) break;
				if( c == 32 ) {
					if (a){
						cout<<i<<endl;
						break;
					}
					a=!a;
				}
				if (a) {
					if (i > 64) {
							cout<<"Too long gesture. unable to process.\n";
							a = false;
							cout<<"Try Again\n\n";
							}
					else {
						gest[i] = writecenter(mid);
						if (gest[i].car!=0 || gest[i].cdr!=0) i++;
					}
				}
	
			}
		//for(q=0; q<i; q++){
		//cout<<"fin["<<q<<"].car = "<<gest[q].car;
		//cout<<"     ; fin["<<q<<"].cdr = "<<gest[q].cdr<<endl;
		//}
		cout<<endl;
		
			
		if(i<N)N = i; 
		scaleimg(gest, i);
		for(j=0; j<N; j++){
			fin[j].car += gest[j].car ;
			fin[j].cdr += gest[j].cdr ;
		}
		
		cvDestroyWindow ( "Recording ...press ESC to stop !");
	}
	
	scaleimg(fin, N);
	//cout<<"yo2\n!!! ... ... !!!\n\n";
	
		
	char fname[10]="xfile.bin";
	fname[0] = g;
	fp = fopen(fname, "wb+");
	
	fwrite(&N,4,1,fp);
	for(i=0 ; i < N ; i++){
		x = fin[i];
		//cout<<x.car<<"  ...  "<<x.cdr<<endl;
		fwrite(&x,8,1,fp);
  }
  fclose(fp);
  //cout<<"yo3\n!!! ... ... !!!\n\n";
	
		
  char fname1[10]="xcmmd.bin";
	fname1[0] = g;
	fp = fopen(fname1, "wb+");
	//cout<<"yo4\n!!! ... ... !!!\n\n";
	
	for(i=0; i<32; i++){
		l = cmmd[i];	
		fwrite(&l,1,1,fp);
	}
	fclose(fp);
  //cout<<"Gesture succesfully saved.\n";
	//cout<<"yo5\n!!! ... ... !!!\n\n";
	
	fp = fopen ("gestures.bin", "rb+");
	for(i=0; i<32; i++){
		fread(&l,1,1,fp);
		str1[i] = l;
	}
	fclose(fp);
	for(i=0;i<31;i++){
		if(str1[i]==g){
			cout<<"Gesture overwritten.\n";
			break;
		}
		if(str1[i]=='\0'){
			str1[i] = g;
			str1[i+1] = '\0';
			cout<<"Gesture succesfully saved.\n";
			break;
		}
	}
	fp = fopen ("gestures.bin", "wb+");
	for(i=0; i<32; i++){
		l = str1[i];	
		fwrite(&l,1,1,fp);
	}
	fclose(fp);
	/*
	for(i=0; i<N; i++){
		cout<<"fin["<<i<<"].car = "<<fin[i].car;
		cout<<"     ; fin["<<i<<"].cdr = "<<fin[i].cdr<<endl;
		}
		*/
	writeimg(fin, N, res);
	char fname2[10]="xgest.jpg";
	fname2[0] = g;
	cvSaveImage(fname2,res);
	//cout<<"yo7\n!!! ... ... !!!\n\n";
	
		
	//cvReleaseImage (&mid);
	//cvReleaseImage (&frames);
	//cvReleaseCapture ( &capture );
	//cvReleaseImage (&res);
	//cvDestroyWindow ( "Recording ...press ESC to stop !");				
	//cout<<"yo8\n!!! ... ... !!!\n\n";
	return 0;
}

