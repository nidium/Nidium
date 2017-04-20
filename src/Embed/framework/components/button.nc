<component name="button">
    <nss>
        button: {
            overflow : false,
            backgroundColor : "#3388ee",
            color : "#ffffff",
            radius : 4,
            height : 40,
            width:80,
            lineHeight : 30,
            coating : 0,

            paddingTop : 6,
            paddingBottom : 6,
            paddingLeft : 10,
            paddingRight : 10,

            fontSize : 14,
            textAlign : "center",
            textOffsetY : 0,

            opacity : 1.0,
            borderWidth : 6,
            borderColor : "rgba(0, 0, 0, 0.003)",

            shadowBlur : 3,
            shadowColor : "rgba(0, 0, 0, 0.12)",
            shadowOffsetX : 0,
            shadowOffsetY : 2
        }
    </nss>
    <layout id="button" class="button">
        <slot>Button</slot>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                console.log("dsqsffd");
            }
        </script>
</component>
