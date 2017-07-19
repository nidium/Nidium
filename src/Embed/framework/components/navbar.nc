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
            lineHeight: 26,
            maxWidth: 26
        },

        menu : {
            flexGrow: 1,
            minWidth:90,
            display:"none"
        },

        big: {
            fontSize: 28,
            width: 30,
            height: 30,
            lineHeight: 29,
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
        <span id="menu" class="flexme menu" on:click="menu(this)">
            <icon class="icon big" shape="ion-navicon-round"></icon>
        </span>
        <span id="left" class="flexme left" on:click="back()">
            <icon class="icon" shape="ion-chevron-left" style="{marginRight:0}"></icon>
            <span class="grow">Back</span>
        </span>
        <span class="flexme center" on:click="center()">
            <span id="title" class="grow"></span>
        </span>
        <span id="right" class="flexme right" on:click="next()">
            <span class="grow">Next</span>
            <icon class="icon" shape="ion-chevron-right" style="{marginLeft:0}"></icon>
        </span>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                this.data = this.attributes.data || {};
                this.title = (this.data.title || attr.title) || "";
            }

            onready() {
                var title = this.getElementById("title");
                title.textContent = this.title;

                var left = this.getElementById("left");
                if (this.attributes['on:back']) {
                    left.show();
                } else {
                    left.hide();
                }

                var right = this.getElementById("right");
                if (this.attributes['on:next']) {
                    right.show();
                } else {
                    right.hide();
                }

                var menu = this.getElementById("menu");
                if (this.attributes['on:menu']) {
                    left.style.display = "none";
                    left.hide();
                    menu.style.display = "flex";
                    menu.show();
                } else {
                    menu.style.display = "none";
                    menu.hide();
                }

            }

            center() {
            }

            menu(e) {
                this.emit("menu", {});
            }

            back(e) {
                this.emit("back", {});
            }

            next(e) {
                this.emit("next", {});
            }
        }
    </script>
</component>
