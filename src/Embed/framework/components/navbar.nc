<component name="navbar">
    <nss>
        bar: {
            backgroundColor: "#3388EE",
            flexDirection: "row",
            flexGrow:1,
            maxHeight: 46,
            lineHeight: 46,
            textAlign: "center",
            color: "#ffffff"
        },

        icon: {
            marginLeft:10,
            marginRight:10,
            fontSize: 20,
            flexGrow:1,
            width: 26,
            height: 30,
            lineHeight: 30,
            maxWidth: 26
        },

        menu : {
            display : "none",
            flexGrow: 1,
            minWidth:90,
        },

        big: {
            fontSize: 28,
            width: 30,
            height: 30,
            lineHeight: 33,
            maxWidth: 30
        },

        flexme: {
            flexDirection: "row",
            alignItems: "center"
        },

        left: {
            flexGrow: 1,
            flexShrink: 0,
            minWidth:90,
            backgroundColor: "rgba(0, 30, 0, 0.0)",
            textAlign:"left"
        },

        center: {
            flexGrow: 4,
            flexShrink: 1,
            alignItems:"center",
            textAlign:"center"
        },

        right: {
            flexGrow: 1,
            flexShrink: 0,
            minWidth:90,
            backgroundColor: "rgba(0, 30, 0, 0.0)",
            textAlign:"right"
        },

        grow : {
            flexGrow:1
        }
    </nss>
    <layout class="bar">
        <span id="menu" class="flexme menu" on:mousedown="this.menu()">
            <icon class="icon big" shape="ion-navicon-round"></icon>
        </span>
        <span id="back" class="flexme left" on:mousedown="this.back()">
            <icon class="icon" shape="ion-chevron-left" style="{marginRight:0}"></icon>
            <span class="grow">Back</span>
        </span>
        <span class="flexme center">
            <span id="title" class="grow">Nidium Kitchen</span>
        </span>
        <span id="next" class="flexme right" on:mousedown="this.next()">
            <span class="grow">Next</span>
            <icon class="icon" shape="ion-chevron-right" style="{marginLeft:0}"></icon>
        </span>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                var node = this.getElementById("title");
                node.textContent = attr.title || "";

                if (attr['on:menu']) {
                    var menu = this.getElementById("menu");
                    menu.style.display = "flex";
                }

                if (!attr['on:back']) {
                    var left = this.getElementById("back");
                    left.style.display = "none";
                }

                if (!attr['on:next']) {
                    var right = this.getElementById("next");
                    right.style.display = "none";
                }
            }

            menu() {
                this.emit("menu", {});
            }

            back() {
                this.emit("back", {});
            }

            next() {
                this.emit("next", {});
            }
        }
    </script>
</component>
