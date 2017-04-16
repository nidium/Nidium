<component name="button">
    <nss>
        button: {
            color : "#ecedeb",
            backgroundColor : "#ffffff",
            overflow : false,

            radius : 2,
            height : 22,
            paddingLeft : 10,
            paddingRight : 10,

            textAlign : "center",
            textOffsetY : 0,

            borderWidth : 10,
            borderColor : "rgba(0, 0, 0, 0.18)",

            shadowBlur : 4,
            shadowColor : "rgba(0, 0, 0, 0.2)",
            shadowOffsetX : 0,
            shadowOffsetY : 2,
        }
    </nss>
    <layout>
        <element id="button" class="button">
            <slot>Button</slot>
        </element>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
            }
        }
    </script>
</component>
