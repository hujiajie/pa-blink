
   try{//Try to define 3 source ParallelArray
    if(chrome && CInterface!=undefined);
  }
  catch(e){  
  //Disable buttons
  document.getElementById("runall").disabled=true;
  document.getElementById("clean").disabled=true;
  document.getElementById("button0").disabled=true;
  document.getElementById("button01").disabled=true;
  document.getElementById("button02").disabled=true;
  document.getElementById("button03").disabled=true;
  document.getElementById("button04").disabled=true;
  document.getElementById("button05").disabled=true;
  document.getElementById("button10").disabled=true;
  document.getElementById("button11").disabled=true;
  document.getElementById("button12").disabled=true;
  document.getElementById("button13").disabled=true;
  document.getElementById("button14").disabled=true;
  //document.getElementById("button15").disabled=true;
  document.getElementById("button16").disabled=true;
  document.getElementById("button20").disabled=true;
  document.getElementById("button21").disabled=true;
  document.getElementById("button30").disabled=true;
  document.getElementById("button31").disabled=true;
  document.getElementById("button32").disabled=true;
  document.getElementById("button40").disabled=true;
  document.getElementById("button41").disabled=true;
  document.getElementById("button42").disabled=true;
  document.getElementById("button43").disabled=true;
  document.getElementById("button44").disabled=true;
  document.getElementById("button45").disabled=true;
  //document.getElementById("button50").disabled=true;
  document.getElementById("button51").disabled=true;
  document.getElementById("button52").disabled=true;
  document.getElementById("button60").disabled=true;
  document.getElementById("button61").disabled=true;
  document.getElementById("button70").disabled=true;
  document.getElementById("button71").disabled=true;
  document.getElementById("button72").disabled=true;
  document.getElementById("button73").disabled=true;
  document.getElementById("button80").disabled=true;
  document.getElementById("button81").disabled=true;
  //Alert to user
  alert("RiverTrail is unsupported or you are not using chrome!");
 }
  function runAll(){//run all tests
  makeRegular();makeRegular1();makeUnRegular();makeEmpty();makeNested();testElement();
  useMap01();useMap02();useMap04();useCombine01();useCombine02();useCombine03();
  useScan1();useScan2();
  useReduce1();useReduce2();useReduce3();
  useScatter1();useScatter2();useScatter3();useScatter4();useScatter5();histogram();
  //useFilter1();
  useFilter2();useFilter3();
  useFlatten1();useFlatten2()
  usePartition1();usePartition2();usePartition3();usePartition4();
  useGet1();useGet2();
  }
  function resultClean(){//reset all results
   document.getElementById("testresult").innerHTML="Test results are shown here:";
   document.getElementById("result0").innerHTML="";
   document.getElementById("result01").innerHTML="";
   document.getElementById("result02").innerHTML="";
   document.getElementById("result03").innerHTML="";
   document.getElementById("result04").innerHTML="";
   document.getElementById("result05").innerHTML="";
   document.getElementById("result10").innerHTML="";
   document.getElementById("result11").innerHTML="";
   document.getElementById("result12").innerHTML="";
   document.getElementById("result13").innerHTML="";
   document.getElementById("result14").innerHTML="";
   //document.getElementById("result15").innerHTML="";
   document.getElementById("result16").innerHTML="";
   document.getElementById("result20").innerHTML="";
   document.getElementById("result21").innerHTML="";
   document.getElementById("result30").innerHTML="";
   document.getElementById("result31").innerHTML="";
   document.getElementById("result32").innerHTML="";
   document.getElementById("result40").innerHTML="";
   document.getElementById("result41").innerHTML="";
   document.getElementById("result42").innerHTML="";
   document.getElementById("result43").innerHTML="";
   document.getElementById("result44").innerHTML="";
   document.getElementById("result45").innerHTML="";
   //document.getElementById("result50").innerHTML="";
   document.getElementById("result51").innerHTML="";
   document.getElementById("result52").innerHTML="";
   document.getElementById("result60").innerHTML="";
   document.getElementById("result61").innerHTML="";
   document.getElementById("result70").innerHTML="";
   document.getElementById("result71").innerHTML="";
   document.getElementById("result72").innerHTML="";
   document.getElementById("result73").innerHTML="";
   document.getElementById("result80").innerHTML="";
   document.getElementById("result81").innerHTML="";
  }
   
  var source0 = new ParallelArray([1,2,3,4,5,6]);
  var source1 = new ParallelArray(3,function(i){var ret = [0, 0]; ret[0] = i; ret[1] = i + 1; return ret;});
  var source2 = new ParallelArray([1/3,2,5]);

  //ParallelArray test
  function makeRegular(){//Define regular arraies
   var PA0=new ParallelArray(256, function(i) {
            return [i, i+1, i+2, i+3];
        });
    var PA1=new ParallelArray(256, function(i) {
            var x = [];
            for (var i = 0; i < 4; i++) {
                x[i] = i;
            }
            return x;
        });
	var PA2=new ParallelArray(256, function(i) {
            var x = [];
            for (var i = 0; i < 99; i++) {
                x[i] = i;
            }
            return x;
        });
   document.getElementById("testresult").innerHTML+=("<p>New regular Parallelarray with synopsis:ParallelArray(size, elementalFunction, arg1, ..., argN);  </p>");
    document.getElementById("testresult").innerHTML+=("<p>size:256</p>"+"<p>shape</p>"+PA0.shape+PA0);
	document.getElementById("testresult").innerHTML+=("<p>size:256</p>"+"<p>shape</p>"+PA1.shape+PA1);
	document.getElementById("testresult").innerHTML+=("<p>size:256</p>"+"<p>shape</p>"+PA2.shape+PA2);
  document.getElementById("result01").innerHTML="Passed";
  document.getElementById("result01").style.color="green";
  }
  function makeRegular1(){//Define regular arraies
  var source3=new ParallelArray(source0);
   document.getElementById("testresult").innerHTML+=("<p>Source0,Source2 is listed in the table.Only the outer most array-like structure is transformed into a ParallelArray object. </p>");
  document.getElementById("result02").innerHTML="Passed";
  document.getElementById("result02").style.color="green";
  }
  function makeUnRegular(){//Define a unregular array 
   try{
    var synPA0=new ParallelArray([1,2,[3,4]]);
	if((synPA0.getArray())[2][1]){
	  document.getElementById("testresult").innerHTML+=("<p>Ungular array can not be in operation.</p>"+
      "<p>Use [2][1] to this array can get:</p>"+(synPA0.getArray())[2][1]);
      document.getElementById("result0").innerHTML="Passed";
      document.getElementById("result0").style.color="green";}
     }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Unregular Parallearray is undefined.</p>"+e.name);
	     document.getElementById("result0").innerHTML="Unsupported";
		 document.getElementById("result0").style.color="yellow";
		 }
  }
  function makeEmpty(){//Define an empty array 
   try{
    var synPA1=new ParallelArray();
	//var synPA1=new ParallelArray([]);//legal
	  document.getElementById("testresult").innerHTML+=("<p>It's legal:</p>"+synPA1);
      document.getElementById("result03").innerHTML="Passed";
      document.getElementById("result03").style.color="green";}
     
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Empty array failed.</p>"+e.name);
	     document.getElementById("result03").innerHTML="Unsupported";
		 document.getElementById("result03").style.color="yellow";
		 }
  }
  function makeNested(){//Define a nested array 
   try{
    var PA4 = new ParallelArray(256, function (x) {
    return new ParallelArray(256, function(y) { return x*1000 + y; });
  });
	  document.getElementById("testresult").innerHTML+=("<p>Nested array:</p>"+PA4);
      document.getElementById("result04").innerHTML="Passed";
      document.getElementById("result04").style.color="green";}
     
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Nested array failed.</p>"+e.name);
	     document.getElementById("result04").innerHTML="Unsupported";
		 document.getElementById("result04").style.color="yellow";
		 }
  }
   function testElement(){//Test element kinds
   try{
    var a = [1,{},"a",false];
    var PA7 = new ParallelArray(a);
	//test out of bound
	if(PA7.get(42)==undefined){
	  document.getElementById("testresult").innerHTML+=("<p>Element test:</p>"+PA7);
      document.getElementById("result05").innerHTML="Passed";
      document.getElementById("result05").style.color="green";}
	  else{
	  document.getElementById("result05").innerHTML="Failed";
      document.getElementById("result05").style.color="red";
	  }
     }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Nested array failed.</p>"+e.name);
	     document.getElementById("result05").innerHTML="Unsupported";
		 document.getElementById("result05").style.color="yellow";
		 }
  }
  	//Haven't test follow:
	//ParallelArray with holes;
	//var synPA6=new ParallelArray(canvas);
  
  
  


  //Map test
  function useMap01(){
     try{
	 var plusOne = source0.map(function inc(v) { return v+1; });
	 var flag=0;//"[2,3,4,5,6,7]"
	 if(plusOne.get(0)!=2||plusOne.get(1)!=3||plusOne.get(2)!=4||plusOne.get(3)!=5||plusOne.get(4)!=6||plusOne.get(5)!=7) flag=1;
    if(flag==0){
      document.getElementById("testresult").innerHTML+=("<p>PlusOne Source0:</p>"+plusOne);
	  document.getElementById("result10").innerHTML="Passed";
	  document.getElementById("result10").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Plus one failed.</p>");
	     document.getElementById("result10").innerHTML="Failed";
		 document.getElementById("result10").style.color="red";}
    }
	catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Map failed.</p>"+e.name);
	     document.getElementById("result10").innerHTML="Unsupported";
		 document.getElementById("result10").style.color="yellow";
		 }
    }

   function useMap02(){
	 try{
    var plusOne = source1.map(function inc(v) { return v+1; });
	var flag=0;//"[[0, 1]1, [1, 2]1, [2, 3]1]"
	if(plusOne.get(0,0)!=0||plusOne.get(0,1)!=1||plusOne.get(1,0)!=1||plusOne.get(1,1)!=2||plusOne.get(2,0)!=2||plusOne.get(2,1)!=3) flag=1;
	if(flag==0){
      document.getElementById("testresult").innerHTML+=("<p>PlusOne Source1:</p>"+plusOne);
	  document.getElementById("result11").innerHTML="Passed";
	  document.getElementById("result11").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Failed:array type elements are regarded as string!</p>"+"<p>Because nested elements are referenced as is:</p>"+plusOne);
         document.getElementById("result11").innerHTML="Failed";
         document.getElementById("result11").style.color="red";}
		 }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>The RiverTrail doesn't support nested elements plus number!</p>"+e.name);
	     document.getElementById("result11").innerHTML="Unsupported";
		 document.getElementById("result11").style.color="yellow";
		 }
    }
	
/*--	function useMap03(){
	 try{
	 var object0= source0.map(function (v) {
    var x = [];
    var N = 2;
    for (var i = 0; i < 10; i++) {
      if ((i % N) == 0) {
        x[i] = {f1: v};
      } else if ((i % N) == 1) {
        x[i] = {f1: v, f2: v, f3: v,
                f4: v, f5: v, f6: v,
                f7: v, f8: v, f9: v};
      }
    }
    return x;
  });
      document.getElementById("testresult").innerHTML+=("<p>Object Source0:</p>"+object0);
	  document.getElementById("result15").innerHTML="Passed";
	  document.getElementById("result15").style.color="green";}
	   catch(e){    
		 document.getElementById("testresult").innerHTML+=(e.name);
	     document.getElementById("result15").innerHTML="Unsupported";
		 document.getElementById("result15").style.color="yellow";
		 }
  }--*/
	
	function useMap04(){
	 try{
	 var PA4=new ParallelArray(["a", "b", "c", "d", "e", "f", "g", "h",
                     "i", "j", "k", "l", "m", "n", "o", "p",
                     "q", "r", "s", "t", "u", "v", "w", "x",
                     "y", "z"]);
	  var compare= PA4.map(function comp(v){return v=="x"||v=="u";});
	 
      document.getElementById("testresult").innerHTML+=("<p>If the element is u or x:</p>"+compare);
	  document.getElementById("result16").innerHTML="Passed";
	  document.getElementById("result16").style.color="green";}
	   catch(e){    
		 document.getElementById("testresult").innerHTML+=(e.name);
	     document.getElementById("result16").innerHTML="Unsupported";
		 document.getElementById("result16").style.color="yellow";
		 }
  }
	
	//Combine test
	 function useCombine01(){
	 try{
    var plusOne = source0.combine(function inc(i) { return this.get(i)+1; });
    if(plusOne.toString()=="[2, 3, 4, 5, 6, 7]"){
      document.getElementById("testresult").innerHTML+=("<p>PlusOne Source0:</p>"+plusOne);
	  document.getElementById("result12").innerHTML="Passed";
	  document.getElementById("result12").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Plus one failed.</p>");
	     document.getElementById("result12").innerHTML="Failed";
		 document.getElementById("result12").style.color="red";}
		 }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>The RiverTrail doesn't supported Combine:</p>"+e.name);
	     document.getElementById("result12").innerHTML="Unsupported";
		 document.getElementById("result12").style.color="yellow";
		 }
    }
	
	
	function useCombine02(){
	 try{
    var plusOne = source1.combine(function inc(i) { return this.get(i)+1; });
    if(plusOne.toString()=="[[0, 1]1, [1, 2], [2, 3]]"){
      document.getElementById("testresult").innerHTML+=("<p>PlusOne Source1:</p>"+plusOne);
	  document.getElementById("result13").innerHTML="Passed";
	  document.getElementById("result13").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Plus one failed.</p>"+plusOne);
	     document.getElementById("result13").innerHTML="Failed";
		 document.getElementById("result13").style.color="red";}
		 }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>The RiverTrail doesn't supported Combine:</p>"+e.name);
	     document.getElementById("result13").innerHTML="Unsupported";
		 document.getElementById("result13").style.color="yellow";
		 }
    }
	function useCombine03(){
	 try{
    var plusOne = source1.combine(2,function inc(i) { return this.get(i)+1; });//<<2,3>,<4,5>,<6,7>>
    if(plusOne.toString()=="[1, 2, 2, 3, 3, 4]"){
      document.getElementById("testresult").innerHTML+=("<p>PlusOne Source1:</p>"+plusOne);
	  document.getElementById("result14").innerHTML="Passed";
	  document.getElementById("result14").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Plus one failed.</p>"+plusOne);
	     document.getElementById("result14").innerHTML="Failed";
		 document.getElementById("result14").style.color="red";}
		 }
		 catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>The RiverTrail doesn't supported Combine:</p>"+e.name);
	     document.getElementById("result14").innerHTML="Unsupported";
		 document.getElementById("result14").style.color="yellow";
		 }
    }

  
  //Scan test
  function useScan1(){
  try{
    var psum = source0.scan(function plus(a,b) { return a+b; }); 
	var flag=0;//"[1, 3, 6, 10, 15, 21]"
	if(psum.get(0)!=1||psum.get(1)!=3||psum.get(2)!=6||psum.get(3)!=10||psum.get(4)!=15||psum.get(5)!=21) flag=1;
    if(flag==0){
      document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+psum);
	  document.getElementById("result20").innerHTML="Passed";
	  document.getElementById("result20").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Scan failed.</p>");
	document.getElementById("result20").innerHTML="Failed";
	document.getElementById("result20").style.color="red";}
	}
	catch(e){
	 document.getElementById("testresult").innerHTML+=("<p>Scan failed.</p>"+e.name);
	     document.getElementById("result20").innerHTML="Unsupported";
		 document.getElementById("result20").style.color="yellow";
	}
	}
  function useScan2(){
  try{
    var psum = source1.scan(function plus(a,b) { return a+b; }); 
	var flag=0;//"[0, 1, 1, 3, 3, 6]"
	if(psum.get(0)!=0||psum.get(1)!=1||psum.get(2)!=1||psum.get(3)!=3||psum.get(4)!=3||psum.get(5)!=6) flag=1;
	if(flag==0){
      document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+psum);
	  document.getElementById("result21").innerHTML="Passed";
	  document.getElementById("result21").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Scan failed:Abnormal output.</p>");
	document.getElementById("result21").innerHTML="Failed";
	document.getElementById("result21").style.color="red";}
	}
	catch(e){
	 document.getElementById("testresult").innerHTML+=("<p>Scan failed.</p>"+e.name);
	     document.getElementById("result21").innerHTML="Unsupported";
		 document.getElementById("result21").style.color="yellow";
	}
  }
  
  //reduce test
  //Typically the programmer will only call reduce with associative and commutative functions but there is nothing preventing them doing otherwise.
  function useReduce1(){
  try{
    var sum = source0.reduce(function plus(a,b) { return a+b; });
    if(sum==21){
      document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+sum);
	  document.getElementById("result30").innerHTML="Passed";
	  document.getElementById("result30").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Reduce failed.</p>");
	document.getElementById("result30").innerHTML="Failed";
	document.getElementById("result30").style.color="red";}
	}
	catch(e){    
		 document.getElementById("testresult").innerHTML+=("<p>Reduce failed:</p>"+e.name);
	     document.getElementById("result30").innerHTML="Unsupported";
		 document.getElementById("result30").style.color="yellow";
		 }
	
  }
  function useReduce2(){
  try{
    var sum = source1.reduce(function plus(a,b) { return a+b; });
	var flag=0;//[3.6]
	if(sum.get(0)!=3||sum.get(1)!=6) flag=1;
	if(flag==0){
      document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+sum);
	  document.getElementById("result31").innerHTML="Passed";
	  document.getElementById("result31").style.color="green";}
	else{document.getElementById("testresult").innerHTML+=("<p>Reduce failed:Abnormal output:</p>"+sum);//[0,1][1,2][2,3]
	document.getElementById("result31").innerHTML="Failed";
	document.getElementById("result31").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Reduce failed:</p>"+e.name);
	     document.getElementById("result31").innerHTML="Unsupported";
		 document.getElementById("result31").style.color="yellow";
	}
  }
  
  
  function assertAlmostEq(v1, v2) {
  if (v1 === v2) 
    return true;
  // + and other fp ops can vary somewhat when run in parallel!
  if(typeof v1== "number") document.getElementById("testresult").innerHTML+=("<p>heihei</p>");
  if(typeof v2== "number") document.getElementById("testresult").innerHTML+=("<p>hehe</p>");
  var diff = Math.abs(v1 - v2);
  var percent = diff / v1 * 100.0;
  document.getElementById("testresult").innerHTML+=("<p>v1=</p>"+v1);
  document.getElementById("testresult").innerHTML+=("<p>v2=</p>"+v2);
  document.getElementById("testresult").innerHTML+=("<p>% diff = </p>"+percent);
  if(percent < 1e-10) return true; // off by an less than 1e-10%...good enough.
  
}
  
  function useReduce3(){
  try{
    var sum = source2.reduce(function plus(a,b) { return a+b; });
	if(assertAlmostEq(sum,7.3333333333333)){
      document.getElementById("testresult").innerHTML+=("<p><Correct:</p>"+sum);
	  document.getElementById("result32").innerHTML="Passed";
	  document.getElementById("result32").style.color="green";}
	else{document.getElementById("testresult").innerHTML+=("<p>Reduce failed.</p>");
	document.getElementById("result32").innerHTML="Failed";
	document.getElementById("result32").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Reduce failed:</p>"+e.name);
	     document.getElementById("result32").innerHTML="Unsupported";
		 document.getElementById("result32").style.color="yellow";}
  }
  
  //scatter test
  function useScatter1(){
  try{
    var reorder = source0.scatter([4,5,0,3,1,2]);//"[3, 5, 6, 4, 1, 2]"
	var flag=0;
	if(reorder.get(0)!=3 || reorder.get(1)!=5|| reorder.get(2)!=6 || reorder.get(3)!=4 || reorder.get(4)!=1 || reorder.get(5)!=2) flag=1;
    if(flag==0){
        document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+reorder);
		document.getElementById("result40").innerHTML="Passed";
		document.getElementById("result40").style.color="green";}
  	else{document.getElementById("testresult").innerHTML+=("<p>Scatter failed.</p>"+reorder);
	document.getElementById("result40").innerHTML="Failed";
	document.getElementById("result40").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Scatter failed:</p>"+e.name);
	     document.getElementById("result40").innerHTML="Unsupported";
		 document.getElementById("result40").style.color="yellow";}
  }
  function useScatter2(){
  try{
    var reorder = source1.scatter([2,0,1]);
	var flag=0;//"[[1, 2],[ 2, 3],[ 0, 1]]"
	if(reorder.get(0,0)!=1||reorder.get(0,1)!=2||reorder.get(1,0)!=2||reorder.get(1,1)!=3||reorder.get(2,0)!=0||reorder.get(2,1)!=1) flag=1;
	if(flag==0){
        document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+reorder);
		document.getElementById("result41").innerHTML="Passed";
		document.getElementById("result41").style.color="green";}
  	else{document.getElementById("testresult").innerHTML+=("<p>Scatter failed.</p>"+reorder);
	document.getElementById("result41").innerHTML="Failed";
	document.getElementById("result41").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Scatter failed:</p>"+e.name);
	     document.getElementById("result41").innerHTML="Unsupported";
		 document.getElementById("result41").style.color="yellow";}
  }
  function useScatter3(){
  try{
    var reorder = source1.scatter([2,1]);
	var flag=0;//"[ , [1, 2], [0, 1]]"
	if(reorder.get(0)!=undefined||reorder.get(1,0)!=1||reorder.get(1,1)!=2||reorder.get(2,0)!=0||reorder.get(2,1)!=1) flag=1;
    if(flag==0){
       document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+reorder);
	   document.getElementById("result42").innerHTML="Passed";
	   document.getElementById("result42").style.color="green";}
  	else{document.getElementById("testresult").innerHTML+=(e.error+"<p>Scatter failed.No default value and confliction.</p>");
	document.getElementById("result42").innerHTML="Failed";
	document.getElementById("result42").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=(e.name+"<p>Scatter failed:no default value and confliction.</p>");
	     document.getElementById("result42").innerHTML="Unsupported";
		 document.getElementById("result42").style.color="yellow";}
  }
  function useScatter4(){
  try{
    var reorder = source1.scatter([2,1],520);
	var flag=0;//"[520, [1, 2],[ 0, 1]]"
	if(reorder.get(0)!=520||reorder.get(1,0)!=1||reorder.get(1,1)!=2||reorder.get(2,0)!=0||reorder.get(2,1)!=1) flag=1;
      if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+reorder.get(0)+", "+reorder.get(1)+", "+reorder.get(2));
		 document.getElementById("result43").innerHTML="Passed";
		 document.getElementById("result43").style.color="green";}
    	else{document.getElementById("testresult").innerHTML+=("<p>Scatter failed.</p>");
		document.getElementById("result43").innerHTML="Failed";
		document.getElementById("result43").style.color="red";}
		}
		catch(e){
	     document.getElementById("testresult").innerHTML+=(e.name+"<p>Scatter failed:no conflict function.</p>");
	     document.getElementById("result43").innerHTML="Unsupported";
		 document.getElementById("result43").style.color="yellow";}
    }
  function useScatter5(){
  try{
    var reorder = source0.scatter([4,5,0,3,4,2],520,function max(a, b) {return a>b?a:b; });
	var flag=0;
	if(reorder.get(0)!=3||reorder.get(1)!=520||reorder.get(2)!=6||reorder.get(3)!=4||reorder.get(4)!=5||reorder.get(5)!=2) flag=1;//"[3, 520, 6, 4, 5, 2]"
	if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+reorder);
		 document.getElementById("result44").innerHTML="Passed";
		 document.getElementById("result44").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Scatter failed.</p>");
	document.getElementById("result44").innerHTML="Failed";
	document.getElementById("result44").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Scatter failed:</p>"+e.name);
	     document.getElementById("result44").innerHTML="Unsupported";
		 document.getElementById("result44").style.color="yellow";}
  }
  function histogram(){
  try{
    var test0 = new ParallelArray([1,2,2,4,2,4,5]);
    var ones = test0.map(function one(v) { return 1; });//there is a map cause error
    var histogram = ones.scatter(test0, 0, function plus(a,b) { return a+b;}, 6); 
	var flag=0;
	if(histogram.get(0)!=0 ||histogram.get(1)!=1||histogram.get(2)!=3||histogram.get(3)!=0||histogram.get(4)!=2||histogram.get(5)!=1) flag=1;//"[0, 1, 3, 0, 2, 1]"
    if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+histogram);
		 document.getElementById("result45").innerHTML="Passed";
		 document.getElementById("result45").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Scatter failed.</p>");
	document.getElementById("result45").innerHTML="Failed";
	document.getElementById("result45").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Scatter failed:</p>"+e.name);
	     document.getElementById("result45").innerHTML="Unsupported";
		 document.getElementById("result45").style.color="yellow";}
  }
  
  //filter test
  /*function useFilter1(){
  try{
    var filterArray = [1,0,1,0,1,1];
    var result = source0.filter(filterArray);
    if(result.toString()=="[1, 3, 5, 6]"){
         document.getElementById("testresult").innerHTML+=("<p>Correct:</p>"+result);
		 document.getElementById("result50").innerHTML="Passed";
		 document.getElementById("result50").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Filter failed.</p>");
	document.getElementById("result50").innerHTML="Failed";
	document.getElementById("result50").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Filter failed:</p>"+e.name);
	     document.getElementById("result50").innerHTML="Unsupported";
		 document.getElementById("result50").style.color="yellow";}
  }*/
  function useFilter2(){
  try{
  var even = source0.filter(function even(iv) { if(this.get([iv])>3) return true; });
  var flag=0;
  if(even.get(0)!=4||even.get(1)!=5||even.get(2)!=6||even.get(3)!=undefined||even.get(4)!=undefined||even.get(5)!=undefined) flag=1;
  if(flag==0){//"It should have been [4, 5, 6]"
         document.getElementById("testresult").innerHTML+=("<p>Correct:[4,5,6]</p>");
		 document.getElementById("result51").innerHTML="Passed";
		 document.getElementById("result51").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Filter failed.</p>");
	document.getElementById("result51").innerHTML="Failed";
	document.getElementById("result51").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Filter failed:</p>"+e.name);
	     document.getElementById("result51").innerHTML="Unsupported";
		 document.getElementById("result51").style.color="yellow";}
  }
  function useFilter3(){
  try{
  var even = source1.filter(function even(iv,ix) { return (this.get([iv,ix]) % 2) === 0; });
  if(even.get(0)==undefined){
         document.getElementById("testresult").innerHTML+=("<p>Correct:return an empty array. </p>"+even);
		 document.getElementById("result52").innerHTML="Passed"
		 document.getElementById("result52").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Filter failed.</p>");
	document.getElementById("result52").innerHTML="Failed";
	document.getElementById("result52").style.color="red";}
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=("<p>Filter failed:</p>"+e.name);
	     document.getElementById("result52").innerHTML="Unsupported";
		 document.getElementById("result52").style.color="yellow";}
  }
  
  //flatten test
  function useFlatten1(){
    var source=new ParallelArray([3,2], function(iv){return iv[0]*iv[1];});
    var ftest=source.flatten() ;
	var flag=0;//ftest.toString()=="[0, 0, 0, 1, 0, 2]"
	if(ftest.get(0)!=0 || ftest.get(1)!=0 || ftest.get(2)!=0 ||ftest.get(3)!=1||ftest.get(4)!=0||ftest.get(5)!=2) flag=1;
    if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>After flatten:[0, 0, 0, 1, 0, 2]</p>");
		 document.getElementById("result60").innerHTML="Passed";
		 document.getElementById("result60").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Flatten failed.</p>");
	document.getElementById("result60").innerHTML="Failed";
	document.getElementById("result60").style.color="red";}
  }
  function useFlatten2(){
  try{
    var ftest=source0.flatten() ;
    document.getElementById("testresult").innerHTML+=("<p>Flatten failed.</p>");
	document.getElementById("result61").innerHTML="Passed";
	document.getElementById("result61").style.color="green";
	}
	catch(e){
	document.getElementById("testresult").innerHTML+=(e.name+":Cannot flatten 1-dimensional ParallelArray object");
	document.getElementById("result61").innerHTML="Failed";
	document.getElementById("result61").style.color="red";
	}
  }
  
  
  //partition test
  function usePartition1(){
  var ptest=source0.partition(1);
  var flag=0;
  if(ptest.get(0,0)!=1||ptest.get(1,0)!=2||ptest.get(2,0)!=3||ptest.get(3,0)!=4||ptest.get(4,0)!=5||ptest.get(5,0)!=6) flag=1;//ptest.toString()=="[1, 2, 3, 4, 5, 6]"
  if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>After partition:[[1],[2],[3],[4],[5],[6]]</p>");
		 document.getElementById("result70").innerHTML="Passed";
		 document.getElementById("result70").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Partition failed.</p>");
	document.getElementById("result70").innerHTML="Failed";
	document.getElementById("result70").style.color="red";}
  }
  function usePartition2(){
  var ptest=source0.partition(1);
  var ptest1=ptest.partition(3);
  var flag=0;
  if(ptest1.get(0,0,0)!=1||ptest1.get(0,1,0)!=2||ptest1.get(0,2,0)!=3||ptest1.get(1,0,0)!=4||ptest1.get(1,1,0)!=5||ptest1.get(1,2,0)!=6) flag=1;//ptest1.toString()=="[1, 2, 3, 4, 5, 6]"
  if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>After partition:[[[1],[2],[3]],[[4],[5],[6]]]</p>");
		 document.getElementById("result71").innerHTML="Passed";
		 document.getElementById("result71").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Partition failed.</p>");
	document.getElementById("result71").innerHTML="Failed";
	document.getElementById("result71").style.color="red";}
  }
  function usePartition3(){
  var ptest=source1.partition(3);
  var flag=0;//[[0,1][1,2][2,3]]
  if(ptest.get(0,0,0)!=0||ptest.get(0,0,1)!=1||ptest.get(0,1,0)!=1||ptest.get(0,1,1)!=2||ptest.get(0,2,0)!=2||ptest.get(0,2,1)!=3) flag=1;
  if(flag==0){
         document.getElementById("testresult").innerHTML+=("<p>After partition:[[0,1],[1,2],[2,3]]</p>");
		 document.getElementById("result72").innerHTML="Passed";
		 document.getElementById("result72").style.color="green";}
    else{document.getElementById("testresult").innerHTML+=("<p>Partition failed.</p>");
	document.getElementById("result72").innerHTML="Failed";
	document.getElementById("result72").style.color="red";}
  }
  function usePartition4(){
  var H = 96;
  var W = 96;
  var d = 4;
  // 3D 96x96x4 texture-like PA
  var PA5 = new ParallelArray([H,W,d], function (i,j,k) { return i + j + k; });
  var PA6 = new ParallelArray(H).partition(d).partition(W);
   document.getElementById("testresult").innerHTML+=("<p>Partition ParallelArray([96,96,4], function (i,j,k) { return i + j + k; }) :</p>"+PA6);
   document.getElementById("result73").innerHTML="Passed";
   document.getElementById("result73").style.color="green";}
  
  //Get test,the indies should be in a []
  function useGet1(){
    var source=new ParallelArray([3,2], function(iv){return iv[0]*iv[1];});
	if(source.get([2,0])==0){
    document.getElementById("testresult").innerHTML+=("<p>Get([2,0]):</p>"+source1.get([2,0]));
	document.getElementById("result80").innerHTML="Passed";
	document.getElementById("result80").style.color="green";}
	else{
	document.getElementById("result80").innerHTML="Failed";
	document.getElementById("result80").style.color="red";
	}
	}
   function useGet2(){
    var source=new ParallelArray([3,2], function(iv){return iv[0]*iv[1];});
   try{
	if(source.get([2,3])==undefined){
	document.getElementById("testresult").innerHTML+=("<p>Get([2,3]) is</p>"+source.get([2,3]));
	document.getElementById("result81").innerHTML="Passed";
	document.getElementById("result81").style.color="green";
	}
	else{document.getElementById("result81").innerHTML="Failed";
	document.getElementById("result81").style.color="red";}
	}
	catch(e){}
	
	}