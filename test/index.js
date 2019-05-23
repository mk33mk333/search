// 测试 search
const { exec } = require('child_process');
var arr = [
        2,1,0,3,3,2,
        1,5,4,2,5,1,
        1,3,1,0,4,5,
        3,1,2,5,2,1,
        2,4,3,1,0,2,
    ];

var bits = [0,0,0,0,0,0];

for(var i=0;i<arr.length;i++){
    var value = arr[i];
    bits[value] = bits[value] | (1 << i);  // 在这 | 和 + 一样
}

console.log(bits.join(','))

exec(`../bin/search -a ${bits.join(',')} -d`,(error,stdout,stderr)=>{
    if(error){
        console.log(error);
    }else{
        console.log(`stdout: ${stdout}`);
        // console.log(`stderr: ${stderr}`);
        if(stdout){
            var lines = stdout.split('\n'); 
            var result = [];
            var path = [];
            lines.forEach(line=>{
                if(line.indexOf("[result::]") == 0){
                    // console.log('result',line);
                    result = parseResult(line.replace('[result::]',''));
                }
                if(line.indexOf("[path::]") == 0){
                    // console.log('path',line);
                    path = parsePath(line.replace('[path::]',''));
                }
            })
            if(result.length &&path.length){
                // 先验证
                // go(arr,path);
                var ret = path.map(item=>{
                    return {x:item[0],y:item[1]}
                })
                console.log(ret);

                // 返回此种形式 [{x:2,y:2},{x:5,y:2}]


            }
        }else{
            console.log('错误',stderr);
        }
    }
})

// 从头按照路径走一遍
function go (inputArr, path){
    function getPos(x,y){
        return parseInt(y)*6 + parseInt(x);
    }
    function print(arr){
        for(var i = 0;i < 5;i++){
            console.log(`${arr[6*i]},${arr[6*i+1]},${arr[6*i+2]},${arr[6*i+3]},${arr[6*i+4]},${arr[6*i+5]}`)
        }
    }
    for(var i = 0; i < path.length - 1; i++){
        // 找到两个位置
        var p1 = getPos(path[i][0],path[i][1])
        var p2 = getPos(path[i+1][0],path[i+1][1])
        var temp = inputArr[p1];
        inputArr[p1] = inputArr[p2];
        inputArr[p2] = temp;
        console.log(path[i],path[i+1],p1,p2);
        print(inputArr);
    }
}


function parsePath (str) {
    var seg = str.split('-').map(s=>{
        return s.slice(1,-1).split(',')
    }).reverse()
    // console.log(seg);
    return seg;
}



function parseResult (str) {
    var numArr = str.split(',').map(s=>{
        return parseInt(s);
    })
    var ret = [];
    for(var i=0;i<30;i++){
        ret[i] = -1;
    }
    numArr.forEach((n,i)=>{
        // console.log(n.toString(2));
        for(var y = 0;y < 5;y++){
            for(var x = 0;x < 6;x++){
                var pos = y*6+x;
                var t = 1 << pos;
                if((n & t) === t){
                    ret[pos] = i;
                }
            }
        }
    })

    for(var i = 0;i < 5;i++){
        console.log(`${ret[6*i]},${ret[6*i+1]},${ret[6*i+2]},${ret[6*i+3]},${ret[6*i+4]},${ret[6*i+5]}`)
    }
    // console.log(ret);
    return ret;
};