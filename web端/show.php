<!DOCTYPE html>
<html>
<head><link rel="stylesheet" type="text/css" href="table.css"/></head>
<body>
<h1>list show</h1>
<?php
error_reporting(E_ALL ^ E_DEPRECATED);   //解决报警级别
// $fh=fopen("1.csv","r");
// while(!feof($fh))
// {
// $arr=fgetcsv($fh);
// if($arr==NULL)
//     break;
// foreach($arr as $key=>$value)
// {echo $value,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";}
// echo "<br>";
// }
// fclose($fh);


$one=mysql_connect("localhost","root","");
if(!$one){
    die('数据库连接失败'.mysql_error());
}
mysql_select_db("test",$one);    //挑选你的数据库
$result=mysql_query("select * from user");

// 输出表格及th
echo "<table  border='2' width='300'>
                <tr>
                <th>username</th><th>pass</th>
                <th>licensetype</th>
                <th>keynum</th>
                </tr>";
// 遍历数据表中的内容
while($row=mysql_fetch_array($result)){
    echo "<tr>";
    // userid === 用户id 与数据表中的id名称对应
    echo "<td>".$row['username']."</td>";
    // username === 用户名称 与数据表中的username名称对应
    echo "<td>".$row['password']."</td>";
    // password === 用户密码 与数据表中的密码对应
    echo "<td>".$row['licensetype']."</td>";
    
    echo "<td>".$row['keynum']."</td>";
    echo "</tr>";
}
echo "</table>";

echo "<table border='2' width='300'>
                <tr>
                <th>keynum</th><th>pid</th>
                </tr>";

$result=mysql_query("select * from user");  //选取序列号为文件名
while($row=mysql_fetch_array($result)){
    $filename=$row['keynum'];
    $filename=$filename.'.csv';
    if(!file_exists($filename))
        continue;
    $file = fopen($filename,'r') or die("Unable to open file!");
    
while ($data = fgetcsv($file)) {    //每次读取CSV里面的一行内容
    ///print_r($data); //此为一个数组，要获得每一个数据，访问数组下标即可
    if($data[1]=="0")   //对于pid为0的不予展示
        continue;         
    echo "<tr>";
    //输出keynum
    echo "<td>".$row['keynum']."</td>";
    //输出pid
    echo "<td>".$data[1]."</td>";
    echo "</tr>";
}  
}

//数据库部分，待更新
// $result=mysql_query("select * from license");
// echo "<table border='2' width='300'>
//                 <tr>
//                 <th>pid</th><th>keynum</th>
//                 </tr>";
// while($row=mysql_fetch_array($result)){
//     echo "<tr>";
//     // userid === 用户id 与数据表中的id名称对应
//     echo "<td>".$row['pid']."</td>";
//     // username === 用户名称 与数据表中的username名称对应
//     echo "<td>".$row['keynum']."</td>";
//     echo "</tr>";
// }
// echo "</table>";
?>

</body>
</html>