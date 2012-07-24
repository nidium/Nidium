/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/

  var lingrad = canvas.createLinearGradient(0,0,100,100);  
  lingrad.addColorStop(0, '#FF0000');  
  lingrad.addColorStop(1, '#00FF00');  

  canvas.fillStyle = lingrad;

  

  canvas.fillRect(10,10,100,100);  