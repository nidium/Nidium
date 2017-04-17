<component name="statusbar">
    <nss>
        clean: {
            backgroundColor: "#03387e",
            height: 20,
            flexDirection: "row",
            flexGrox: 1,
            color: "#ffffff",
            fontSize: 10,
            lineHeight: 20
        },

        left: {
            flexGrow: 1,
            textAlign: "left"
        },

        center: {
            flexGrow: 3,
            textAlign: "center"
        },

        right: {
            flexGrow: 1,
            textAlign: "right"
        }
    </nss>
    <layout class="clean">
        <span class="left">.:</span>
        <span id="time" class="center"></span>
        <span class="right">:.</span>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                console.log("--->", this.style.backgroundColor, this.height);

                this.timer = null;
                this.timeNode = this.getElementById("time");
                this.tick();
            }

            start() {
                this.stop();
                this.timer = setInterval(()=>{
                    this.tick();
                }, 1000);
            }

            stop() {
                clearInterval(this.timer);
            }

            tick() {
                var now = new moment();
                var text = now.format("HH:mm:ss");

                this.timeNode.textContent = text;
            }

            onmount() {
                this.start();
            }
        }
    </script>
</component>
