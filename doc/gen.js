function ab2str(buf) {
    return String.fromCharCode.apply(null, new Uint8Array(buf));
}

String.prototype.repeat = function( num )
{
    return new Array( num + 1 ).join( this );
}

var f = new File("./core.json");
f.open("r", function() {
    f.read(f.filesize, function(data) {
        var obj = JSON.parse(ab2str(data));
        var str = '';

        str += obj.object + "\n" + "=".repeat(obj.object.length) + "\nLorem ipsum\n";
        str += "##Functions list\n\n";

        for (let i = 0; i < obj.functions.length; i++) {
            str += "###" + obj.functions[i][0] + "\n";
        }
        
        echo(str);
    });
});




