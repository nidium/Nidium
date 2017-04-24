<component name="spinner">
    <layout class="default"></layout>
    <script>

        var scrollArray = function(arr, d){
            if (!d) return arr;
            return arr.slice(d, arr.length).concat(arr.slice(0, d));
        };

        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                this.layout = {};
                this.frame = 0;
                this.loop = true;
                this.playing = false;
                this.paused = false;
                this.forward = true;

                this.style.width = attr.width || 26;
                this.style.height = attr.height || 26;
                this.style.color = attr.color || "#000000";
                this.style.dashes = attr.dashes || 20;
                this.style.radius = attr.radius || 9;
                this.style.speed = attr.speed || 30;
                this.style.lineWidth = attr.lineWidth || 6;

                this.render();
                this.play();
            }

            refresh() {
                var dim = this.getDimensions();

                var width = this.style.lineWidth,
                    height = Math.min(dim.height/2, dim.width/2),
                    radius = Math.min(this.style.radius, height-width);

                this.layout = {
                    opacities : this.getOpacityArray(),
                    top : radius,
                    left : 0-width/2,
                    width : width,
                    height : height - radius,
                    radius : Math.min(height-radius, width) / 2
                };
            }

            getOpacityArray() {
                var array = [],
                    step  = 0.9 / this.style.dashes;

                for (var i=0; i<this.style.dashes; i++) {
                    array.push((i+1) * step);
                }

                return array;
            }

            restart() {
                var speed = this.style.speed,
                    ms = Math.max(16, Math.round(1000/speed));

                clearInterval(this.timer);

                this.timer = setInterval(() => {
                    if (!this.playing) return false;

                    if (this.forward) {
                        this.nextFrame();
                    } else {
                        this.previousFrame();
                    }

                    this.requestPaint();
                }, ms);

            }

            play() {
                if (this.playing) return this;
                if (!this.paused) {
                    this.restart();
                }
                this.playing = true;
                this.paused = false;
            }

            pause() {
                if (!this.playing || this.paused) return this;
                this.playing = false;
                this.paused = true;
            }

            stop() {
                this.frame = 0;
                this.playing = false;
                this.paused = false;
                clearInterval(this.timer);
                return this;
            }

            destroy() {
                this.stop();
                this.removeFromParent();
            }

            nextFrame() {
                if ((++this.frame) >= this.style.dashes){
                    if (this.loop){
                        this.frame = 0;
                    } else {
                        this.stop();
                    }
                }
                return this;
            }

            previousFrame() {
                if ((--this.frame) < 0){
                    if (this.loop){
                        this.frame = this.style.dashes-1;
                    } else {
                        this.stop();
                    }
                }
                return this;
            }

            paint(ctx, width, height) {
                this.refresh();

                var layout = this.layout;
            
                var radian = function(degrees){
                    return (degrees*Math.PI) / 180;
                };

                var opacities = scrollArray(layout.opacities, -this.frame),
                    angle  = radian(360 / this.style.dashes);

                ctx.save();
                ctx.fillStyle = this.style.color;
                ctx.translate(width/2, height/2);

                for (var i=0; i < this.style.dashes; i++) {
                    ctx.globalAlpha = opacities[i];
                    ctx.fillRect(
                        layout.left,
                        layout.top,
                        layout.width,
                        layout.height,
                        layout.radius
                    );
                    ctx.rotate(angle);
                }

                ctx.restore();
            }
        }
    </script>
</component>
