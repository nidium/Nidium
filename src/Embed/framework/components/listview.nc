<component name="listview">
    <nss>
        list: {
            backgroundColor: "#3388EE",
            flexDirection: "column",
            flexGrow:1,
            color: "#000000",
            fontSize: 13,
            lineHeight: 46,
            textAlign: "left",
            overflow:false
        },

        icon: {
            fontSize: 24,
            lineHeight: 24,
            flexGrow:1,
            width: 24,
            maxWidth: 24
        },

        left: {
            flexGrow:1,
            backgroundColor:"#888888"
        },

        right: {
            flexGrow:1
        },

        title : {
            backgroundColor : "blue",
            flexGrow:4
        },

        li : {
            height : 46
        }
    </nss>
    <layout class="list">
        <li class="li">
            <icon class="icon left" shape="fa-chevron-left"></icon>
            <span class="title">Demo</span>
            <icon class="icon right" shape="fa-chevron-right"></icon>
        </li>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
console.log(attr.data)
                window._onmousewheel = (e) => {
                    this.scrollTop += -(2*e.yrel);
//                    console.log(e.yrel);
                };

            }
        }
    </script>
</component>
