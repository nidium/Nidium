<component name="navbar">
    <nss>
        clean: {
            background: "white",
            height: "46px",
            flex: ""
        },

        left: {
            width: "25%"
        },

        center: {
            width: "50%"
        },

        right: {
            width: "25%"
        }
    </nss>
    <template>
        <section id="navbar" class="clean">
            <div class="left">
                <link onclick="backClick()"><i class="icon ion-back" /> <text>Back</text></link>
            </div>
            <div class="center">{{ title }}</div>

            <div class="right">
                
            </div>
        </section>
    </template>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
                this.title = attr.title || "";
            }

            backClick() {
                console.log("clicked");
            }
        }
    </script>
</component>
