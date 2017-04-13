<component name="button">
    <nss>
        button: {
            color : "#ecedeb",
            backgroundColor : "#262722",
            overflow : false,

            radius : 2,
            height : 22,
            paddingLeft : 10,
            paddingRight : 10,

            textAlign : "center",
            textOffsetY : 0,

            borderWidth : 1,
            borderColor : "rgba(0, 0, 0, 0.03)",

            shadowBlur : 4,
            shadowColor : "rgba(0, 0, 0, 0.2)",
            shadowOffsetX : 0,
            shadowOffsetY : 2
        }
    </nss>
    <template>
        <element id="button" class="button">
            <slot>Submit</slot>
        </element>
    </template>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);
            }
        }
    </script>
</component>
