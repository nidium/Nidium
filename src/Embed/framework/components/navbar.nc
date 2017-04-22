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
            fontSize: 24,
            lineHeight: 24,
            flexGrow:1,
            width: 24,
            maxWidth: 24
        },

        il: {
            textAlign: "left",
        },

        ir: {
            textAlign: "right"
        },

        flexme: {
            flexDirection: "row",
            alignItems: "center"
        },

        left: {
            flexGrow: 1,
            flexShrink: 0,
            minWidth:90,
            backgroundColor: "rgba(0, 30, 0, 0.0)"
        },

        center: {
            flexGrow: 4,
            flexShrink: 1,
            alignItems:"center"
        },

        right: {
            flexGrow: 1,
            flexShrink: 0,
            minWidth:90,
            backgroundColor: "rgba(0, 30, 0, 0.0)"
        },

        icleft : {
            textAlign:"left",
            flexGrow:1
        },

        iccenter : {
            textAlign:"center",
            flexGrow:1
        },

        icright : {
            textAlign:"right",
            flexGrow:1
        }
    </nss>
    <layout class="bar">
        <span id="back" class="flexme left" on:mousedown="this.back()">
            <icon class="icon il" shape="fa-chevron-left"></icon>
            <span id="backLabel" class="icleft">Back</span>
        </span>
        <span class="flexme center">
            <span id="title" class="iccenter">Nidium Kitchen</span>
        </span>
        <span id="next" class="flexme right" on:mousedown="this.next()">
            <span class="icright">Next</span>
            <icon class="icon ir" shape="fa-chevron-right"></icon>
        </span>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                var node = this.getElementById("title");
                node.textContent = attr.title || "";

                if (!attr || !attr['on:back']) {
                    var left = this.getElementById("back");
                    left.hide();
                }

                if (!attr || !attr['on:next']) {
                    var left = this.getElementById("next");
                    left.hide();
                }
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
