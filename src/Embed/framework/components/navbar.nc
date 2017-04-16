<component name="navbar">
    <nss>
        clean: {
            backgroundColor: "#3388EE",
            height: "46px",
            width: "100%",

            color: "#ffffff",
            fontSize: 13,
            lineHeight: 46,
            textAlign: "center"
        },

        icon: {
            marginLeft:10,
            fontSize: 24,
            lineHeight: 48,
            width: 30,
            height: "100%",
            color: "#ffffff"
        },

        left: {
            width: "25%",
            height: "100%",
            backgroundColor: "rgba(0, 30, 0, 0.1)"
        },

        icleft : {
            textAlign:"left",
            width:"auto"
        },

        center: {
            width: "50%",
            height: "100%",
        },

        right: {
            width: "25%",
            height: "100%",
            backgroundColor: "rgba(0, 30, 0, 0.1)"
        },

        ar: {
            position: "relative",
            textAlign: "right",
            right:0
        }
    </nss>
    <layout class="clean">
        <span id="a" class="left" on:mousedown="clicked()">
            <icon js:data="{font:font}" on:toto="g(4)" class="icon" shape="fa-chevron-left"></icon> <span class="icleft">Back</span>
        </span>
        <span id="b" class="center">Nidium Kitchen</span>
        <span id="c" class="right">
            <icon js:data="{font:font}" on:mousedown="" class="icon ar" shape="fa-question-circle"></icon>
        </span>
    </layout>
    <script>

        var clicked = function(){
            console.log("fdssdf");
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
