<component name="icon">
    <nss>
        icon: {
            width: 20,
            height: 20,
            fontFamily: "fontAwesome"
        }
    </nss>
    <layout class="icon">
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                this.shape = attr.shape || "";

                if (!this.style.lineHeight) {
                    this.style.lineHeight = this.style.height+1;
                }
            }

            paint(ctx, width, height) {
                super.paint(ctx, width, height);

                let ox = 0;
                let icon = window.fontShapes[this.shape];
                let w = ctx.measureText(icon).width;

                let lineHeight  = this.style.lineHeight;
                let oy = Math.ceil(lineHeight*0.5);

                if (this.style.textAlign == "center") {
                    ox = (width-w)*0.5;
                }

                if (this.style.textAlign == "right") {
                    ox = (width-w);
                }

                ctx.fontFamily    = this.style.fontFamily;
                ctx.fontSize      = this.style.fontSize;
                ctx.fillStyle     = this.style.color;
                ctx.textBaseline  = "middle";
                
                ctx.fillText(icon, ox, oy);
            }
        }
    </script>
</component>
