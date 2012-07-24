/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/


   var lineJoin = ['round','bevel','miter'];  
  canvas.lineWidth = 10;  
  for (var i=0;i<lineJoin.length;i++){  
    canvas.lineJoin = lineJoin[i];  
    canvas.beginPath();  
    canvas.moveTo(-5,5+i*40);  
    canvas.lineTo(35,45+i*40);  
    canvas.lineTo(75,5+i*40);  
    canvas.lineTo(115,45+i*40);  
    canvas.lineTo(155,5+i*40);  
    canvas.stroke();  
  }  