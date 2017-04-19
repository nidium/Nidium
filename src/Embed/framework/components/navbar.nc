<component name="navbar">
    <nss>
        bar: {
            backgroundColor: "#3388EE",
            flexDirection: "row",
            flexGrow:1,
            maxHeight: 46,
            fontSize: 13,
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
            backgroundColor: "rgba(250, 30, 0, 0.5)"
        },

        center: {
            flexGrow: 4,
            alignItems:"center"
        },

        right: {
            flexGrow: 1,
            backgroundColor: "rgba(0, 30, 0, 0.05)"
        },

        icleft : {
            textAlign:"left",
            flexGrow:1,
            backgroundColor:"red"
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
        <span id="back" class="flexme left" on:mouseclick="this.clicked()" on:dblclick="this.dbl()" on:holdstart="this.holdstart()" on:holdend="this.holdstart()" on:mousedown="this.back()">
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

                var back = this.getElementById("back");

                back.on("mousedown", function(){
                    let {width, height} = back.getDimensions();
                    back.style.backgroundColor = "red";
                    console.log("back element:", back.id, back.width, back.height, width, height);

                    console.log(textnode.width, textnode.height)
                });

                var backLabel = this.getElementById("backLabel");
                var textnode = backLabel.getChildren()[0];
                textnode.style.backgroundColor = "green";

                /*
                var g = this.getDimensions();
                console.log(back.width, back.height, g.width, g.height);
                */


                if (!attr || !attr['on:back']) {
                    var left = this.getElementById("back");
                    left.hide();
                }

                if (!attr || !attr['on:next']) {
                    var left = this.getElementById("next");
                    left.hide();
                }
            }

            clicked() {
                //console.log("mouse click");
            }

            dbl() {
                //console.log("mouse double click");
            }

            holdstart() {
                //console.log("mouse hold started");
            }

            holdend() {
                //console.log("mouse hold released");
            }

            back() {
                console.log("BACK")
                this.emit("back", {});
            }

            next() {
                this.emit("next", {});
            }
        }
    </script>
</component>
