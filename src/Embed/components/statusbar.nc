<component name="statusbar">
    <nss>
        clean: {
            backgroundColor: "#212326",
            height: "20px",
            width: "100%",
            color: "#ffffff",
            fontSize: 11,
            lineHeight: 20
        },

        left: {
            width: "25%",
            textAlign: "left"
        },

        center: {
            width: "50%",
            textAlign: "center"
        },

        right: {
            width: "25%",
            textAlign: "right"
        }
    </nss>
    <layout>
        <element class="clean">
            <span class="left">.:</span>
            <span id="time" class="center">{{ time }}</span>
            <span class="right">:.</span>
        </element>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
                console.log(JSON.stringify(attr.data));

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
                console.log("mounted", this.id);
            }
        }
    </script>
</component>
