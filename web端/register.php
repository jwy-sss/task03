
<?php
error_reporting(E_ALL ^ E_DEPRECATED);   //解决报警级别
header("Content-type: text/html; charset=utf-8");
$one=mysql_connect("localhost","root","");
if(!$one){
    die('数据库连接失败'.mysql_error());
}
mysql_select_db("test",$one);    //挑选你的数据库


//接收数据
$username=$_POST['u'];
$password=$_POST['p'];
$licensetype=$_POST['l'];

//从数据库中查找用户名数据
$sql="select*from user where username='$username'";
$check=mysql_query($sql);     //执行查询
$flush= mysql_fetch_array($check);   //返回一个数组

if(is_array($flush))
{
    echo"用户名重复<a href='register.html'>请重新注册</a>";
}
else
{
    //创建前先判断user表是否已经存在
    if(mysql_num_rows(mysql_query("SHOW TABLES LIKE '". 'user'."'"))==1) {
        
        echo "Table exists";
        
    } else {
        
        echo "Table does not exist";
        
        
        $sql = "CREATE TABLE user
        (
            username varchar(15),
            password varchar(15),
        licensetype smallint(1),
        keynum char(10)
        )";
        $check=mysql_query($sql);     //执行创表操作
    }
    $sql="insert into user (username,password,licensetype,keynum) values ('$username','$password','$licensetype','')";   /*插入一条新纪录*/
    $result=mysql_query($sql);    //判断数据插入是否成功
    if(!$result)
    {
        echo"注册不成功<a href='register.html'>点击返回注册界面重新注册</a>";
        
    }
    else
    {
        echo"注册成功<a href='login.html'>返回登录</a>";
        
    }
}
// 关闭数据库
mysql_close($one);
//注册成功后跳转至序列号分发页面
//$r="1334.csv";
//$myfile = fopen("$r", "a") or die("Unable to open file!");//新建文件夹用来储存username
//fwrite($myfile, $username."#".$password."#".$licensetype);//把注册的用户名和密码写入文件
//exit('注册成功.<a href="show.php">返回登陆</a>');

