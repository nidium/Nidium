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
        <span id="menu" class="flexme menu" on:click="menu(this)">
            <icon class="icon big" shape="ion-navicon-round"></icon>
        </span>
        <span id="back" class="flexme left" on:click="back()">
            <icon class="icon" shape="ion-chevron-left" style="{marginRight:0}"></icon>
            <span class="grow">Back</span>
        </span>
        <span class="flexme center" on:click="center()">
            <span id="title" class="grow"></span>
        </span>
        <span class="flexme right" on:click="next()">
            <span id="nextLabel" class="grow">Next</span>
            <icon id="nextIcon" class="icon" shape="ion-chevron-right" style="{marginLeft:0}"></icon>
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

                if (this.attributes['on:menu']) {
                    var menu = this.getElementById("menu");
                    menu.style.display = "flex";
                }

                if (!this.attributes['on:back']) {
                    var left = this.getElementById("back");
                    left.style.display = "none";
                }

                if (!this.attributes['on:next']) {
                    var nextLabel = this.getElementById("nextLabel");
                    nextLabel.style.display = "none";

                    var nextIcon = this.getElementById("nextIcon");
                    nextIcon.style.display = "none";
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
