<component name="button">
    <nss>
        red: {
            background: "red",
        },
        colorFromJS: {
            background: color // Retrieved from JavaScript scope
        }
    </nss>
    <template>
        <div class="red colorFromJS">{{ this.label }}</div>
    </template>

    <script>
        var color = "yellow";

        module.exports = class extends Component {
            constructor(attributes) {
                super(attributes);
                this.label = "Submit";
            }
        }

    </script>
</component>
