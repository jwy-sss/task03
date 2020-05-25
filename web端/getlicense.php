<?php
error_reporting(E_ALL ^ E_DEPRECATED);   //解决报警级别
header("Content-type: text/html; charset=utf-8");
$one=mysql_connect("localhost","root","");
if(!$one){
    die('数据库连接失败'.mysql_error());
}
mysql_select_db("test",$one);    //挑选你的数据库

//接收数据
if ( ( $_POST['u'] != null ) && ( $_POST['p'] != null ) ) {//判断输入是否为空
$username=$_POST['u'];
$password=$_POST['p'];

//从数据库中查找用户名数据
$sql="select*from user where username='$username' and password='$password'";
$check=mysql_query($sql);     //执行查询
$flush= mysql_fetch_array($check);   //返回一个数组

if(!is_array($flush))
{
    echo"用户名或密码输入错误<a href='login.html'>请重新登陆</a>";
}
else
{
   
    $times=10;
    if($flush['licensetype']=='1')
    {
        $times=10;
    }
    else if($flush['licensetype']=='2'){
        $times=20;
    }
    echo "请记好您的许可证";
    echo  $code = rand(1000000000, 9999999999);
    
    $sql="update user set keynum='$code' where username='$username' and password='$password'";
    $check=mysql_query($sql);     //执行更新
    if(!$check)
        echo "插入许可证失败";
    
       
           //创建csv文件存储
        $fp = fopen($code.'.csv', 'w');
        for($count=1;$count<=$times;$count++)
        {fputcsv($fp,array($count,0));

        }
        fclose($fp);  
        
        //创建一张R表用于存储数字和pid,keynum 
        //创建前先判断表是否已经存在
        if(mysql_num_rows(mysql_query("SHOW TABLES LIKE '". 'R'."'"))==1) {
            
            //echo "Table exists";
            
        } else {
            
            //echo "Table does not exist";
            
        
        $sql = "CREATE TABLE R
        (
            num int,
            pid varchar(20),
        keynum char(10)
        )";
     $check=mysql_query($sql);     //执行创表操作
        }
     $count=1;  //设定编号
     for($count;$count<=$times;$count++)
     {
    $sql="insert into R(num,pid,keynum) values ('$count','0','$code')";
    $check=mysql_query($sql);     //执行更新
    if(!$check)
      echo "插入许可证失败";
     }
}
}
else {
    echo "输入不能为空<a href='register.html'>请重新登陆</a>";

}

