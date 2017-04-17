<component name="navbar">
    <nss>
        clean: {
            backgroundColor: "#3388EE",
            flexDirection: "row",
            flexGrow:1,
            maxHeight: 46,
            color: "#ffffff",
            fontSize: 13,
            lineHeight: 46,
            textAlign: "center"
        },

        icon: {
            marginLeft:10,
            marginRight:10,
            fontSize: 24,
            lineHeight: 24,
            flexGrow:1,
            width: 24,
            color: "#ffffff",
            maxWidth: 24
        },

        il: {
            textAlign: "left"
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
            backgroundColor: "rgba(0, 30, 0, 0.05)"
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
    <layout class="clean">
        <span id="a" class="left flexme" on:mousedown="back()">
            <icon js:data="{font:font}" class="icon il" shape="fa-chevron-left"></icon>
            <span class="icleft">Back</span>
        </span>
        <span id="b" class="center flexme">
            <span class="iccenter">Nidium Kitchen</span>
        </span>
        <span id="c" class="right flexme" on:mousedown="next()">
            <span class="icright">Next</span>
            <icon js:data="{font:font}" on:mousedown="" class="icon ir" shape="fa-chevron-right"></icon>
        </span>
    </layout>
    <script>

        var back = function(){
            console.log("back");
        };

        var next = function(){
            console.log("next");
        };

        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
            }

            kk() {

            }
        }
    </script>
</component>
