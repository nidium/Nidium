<component name="navbar">
    <nss>
        clean: {
            backgroundColor: "#3388EE",
            height: "46px",
            width: "100%",

            color: "#ffffff",
            fontSize: 14,
            lineHeight: 46,
            textAlign: "center"
        },

        left: {
            width: "25%",
            height: "100%",
            backgroundColor: "red",
        },

        center: {
            width: "50%",
            height: "100%",
        },

        right: {
            backgroundColor: "#564564",
            width: "25%",
            height: "100%",
        }
    </nss>
    <layout>
        <section class="clean">
            <element id="a" class="left">Back 1</element>
            <element id="b" class="center">Back 2</element>
            <element id="c" class="right">Back 3</element>
        </section>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
            }
        }
    </script>
</component>
