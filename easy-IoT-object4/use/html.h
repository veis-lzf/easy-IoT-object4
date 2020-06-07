#ifndef __HTML_H
#define __HTML_H

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";


//登录界面
const unsigned char html1[] ="\
    <!DOCTYPE html>\
    <html>\
    <head>\
    <title>登录界面</title>\
    </head>\
    <body>\
    <form action=\"sniper\"\
    style=\"margin-top:500px;margin-left:100px;\
    width:500px;height:350px;background-color:darkgray;\
    padding-top:30px;padding-left:70px;\">\
    <div style=\"padding-left:80px;\">\
    <h2 style=\"font-size:35px;\">用户登录</h2></div>\
    <span style=\"font-size:26px;\">用户名:</span>\
      <input type=\"text\" name=\"user\" value=\"admin\" length=\"20\" style=\"width:300px;height:35px;\"/><br><br>\
      <span style=\"font-size:26px;\">密&nbsp;&nbsp;&nbsp;码:</span>\
      <input type=\"password\" name=\"pwd\" value=\"\" style=\"width:300px;height:35px;\"/><br><br>\
      <div style=\"margin-left:80px;margin-height=120px\">\
      <input type=\"reset\" value=\"重置\" style=\"width:80px;height:35px;font-size:20px;margin-right:50px;border-radius:5px;\"/>\
      <input type=\"submit\" value=\"登录\"style=\"width:80px;height:35px;font-size:20px;border-radius:5px;\"/>\
      </div>\
    </form>\
    </body>\
    </html>";

////路由设置界面    
const unsigned char html3[]="\
    <!DOCTYPE html>\
    <html>\
    <head>\
    <title>登录界面</title>\
    </head>\
    <style type=\"text/css\">\
    input{\
      height:27px;\
    }\
    </style>\
    <body>\
    <form action=\"sniper\" style=\"margin-top:500px;margin-left:100px;\
     width:500px;height:350px;background-color:darkgray;\
    padding-top:30px;padding-left:70px;font-size:24px;\">\
       <br><br>\
    SSID:&nbsp;&nbsp;<input type=\"text\" name=\"user\" value=\"\"/><br><br>\
    密码:     <input type=\"password\" name=\"pwd\" value=\"\"/><br><br>\
    <input type=\"reset\" value=\"重置\" style=\"font-size:22px;height:35px;margin-left:50px;\"/>\
    <input type=\"submit\" value=\"保存\" style=\"font-size:22px;height:35px;margin-left:50px;\"/>\
    </form>\
    </body>\
    </html>";




//const unsigned char html4[]="\
//    <!DOCTYPE html>\
//    <html>\
//    <head>\
//    <title>登录界面</title>\
//    </head>\
//    <body>\
//    <form action=\"sniper\" style=\"margin-top:200px;margin-left:650px;\
//    width:500px;height:350px;background-color:darkgray;\
//    padding-top:50px;padding-left:100px;font-size:24px;\"><br><br>\
//    IP&nbsp;&nbsp;&nbsp;名:<input type=\"text\" name=\"ipname\" value=\"\"\
//    style=\"height:30px;\"/><br><br>\
//    端口号:<input type=\"text\" name=\"port\" value=\"\" style=\"height:30px;\"/><br><br>\
//    <input type=\"submit\" value=\"保存\" style=\"height:35px;font-size:22px;margin-left:50px;\"/>\
//    <input type=\"reset\" value=\"重置\" style=\"height:35px;font-size:22px;margin-left:50px;\"/>\
//    </form>\
//    </body>\
//    </html>";

//const unsigned char html5[]="\
//    <!DOCTYPE html>\
//    <html>\
//    <head>\
//    <title>登录界面</title>\
//    </head>\
//    <style type=\"text/css\">\
//    body{\
//       margin:auto 0;\
//      padding:0;\
//     font-size:20px;\
//    }\
//    form{\
//     width:440px;\
//     height:250px;\
//     background:darkgray;\
//    margin-left:35%;\
//    margin-top:10%;\
//    padding-left:200px;\
//    padding-top:80px;\
//      }\
//    #save{\
//      margin-left:70px;\
//     font-size:20px;\
//    height:35px;\
//    }\
//     #clear{\
//       margin-left:50px;\
//      font-size:20px;\
//    height:35px;\
//    }\
//    input{\
//      height:25px;\
//    }\
//    </style>\
//    <body>\
//    <form action=\"sniper\" >\
//    输入原密码:&nbsp;<input type=\"password\" name=\"oldPSD\" value=\"\"/><br><br>\
//    输入新密码:&nbsp;<input type=\"password\" name=\"newpsd\" value=\"\"/><br><br>\
//    确认新密码:&nbsp;<input type=\"password\" name=\"newpsd2\" value=\"\"/><br><br>\
//    <input type=\"submit\" value=\"保存\" id=\"save\"/>\
//    <input type=\"reset\" value=\"重置\" id=\"clear\"/>\
//    </form>\
//    </body>\
//    </html>";

    
//路由设置完成界面
const unsigned char html6[] ="\
    <!DOCTYPE html>\
    <html>\
    <head>\
    <title>路由设置完成界面</title>\
    </head>\
    <body>\
    <form action=\"sniper\"\
    style=\"margin-top:500px;margin-left:200px;\
    width:500px;height:350px;background-color:darkgray;\
    padding-top:30px;padding-left:70px;\">\
    <div style=\"padding-left:80px;\">\
    <h2 style=\"font-size:35px;\">路由设置成功</h2></div>\
    </div>\
    </form>\
    </body>\
    </html>";

    
#endif
