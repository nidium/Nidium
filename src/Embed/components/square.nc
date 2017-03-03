<component name="square">
    <nss>
        blue : {
            background : "blue",
            width : 100,
            height : 100
        }
    </nss>
    <template>
        <element id="square" class="blue">{{ this.componentString }}</element>
    </template>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                this.componentString = attr.label || "";

                this.addEventListener("ready", () => {
                    // DOM is only available after ready event
                    if (attr.color) {
                        let el = this.shadowRoot.getElementById("square");
                        el.style.background = "yellow";
                    }
                });
            }
        }
    </script>
</component>
