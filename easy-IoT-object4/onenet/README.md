# adapter edp lib on LwIP,  V1
# by 老衲五木(qq:540535649). 
# on 2015-02-03
# for detail of edp, please refer to: http://www.heclouds.com/static/files/OneNetAccessPlanAndInterface_v1.1.2.pdf

#file details
1)  Common.h    common data define for Lib

2)  cJSON.c &
	cJSON.h     code for json parser

3)  EdpKit.c &
	EdpKit.h    edp package functions

4)  edp_demo.c  demo for edp package send and receive, also contains code for connection setup to OneNet server

#how to use
    
add Common.h, cJSON.h, cJSON.c, EdpKit.h, EdpKit.c into your project, and compile.
