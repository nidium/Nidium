<component name="statusbar">
    <nss>
        clean: {
            backgroundColor: "#03387e",
            height: 20,
            lineHeight: 20,
            flexDirection: "row",
            color: "#ffffff",
            fontSize: 10
        },

        left: {
            flexGrow: 1,
            textAlign: "left"
        },

        center: {
            flexGrow: 3,
            textAlign: "center"
        },

        right: {
            flexGrow: 1,
            textAlign: "right"
        }
    </nss>
    <layout class="clean">
        <span class="left">.:</span>
        <span id="time" class="center"></span>
        <span class="right">:.</span>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
            }
        }
    </script>
</component>
