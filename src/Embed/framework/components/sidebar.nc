<component name="sidebar">
    <nss>
        bar: {
            backgroundColor: "#282728",
            position: "relative",
            height: "100%",
        },

        flex: {
            display: "flex",
            flexDirection: "column",
            flexGrow: 1,
            justifyContent: "flex-start",
            alignItems: "stretch",
        }
    </nss>
    <layout class="bar">
        <slot class="flex"></slot>
    </layout>
    <script>

        const __opacity_lo__ = 0.4;
        const __opacity_hi__ = 1.0;
        const __next_duration__ = 450;
        const __back_duration__ = 420;

        const Elements = require('Elements');

        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                var sidewidth = attr.width || 320;

                this.style.left = -sidewidth;
                this.style.width = sidewidth;
                this.style.backgroundColor = "red"
            }

            attach(view) {
                this.view = view;
                this.createViewOverlay();
            }

            createViewOverlay() {
                if (this.view && this.view.__overlay__) return false;

                var overlay = new Elements.overlay({
                    opacity : 0.4
                });

                overlay.on("mousedown", () => {
                    this.close();
                });

                this.view.add(overlay);

                this.view.__overlay__ = overlay;
                this.view.__overlay__.style.coating = 40;
                this.view.__overlay__.style.shadowBlur = 20;
                this.view.__overlay__.style.shadowColor = "rgba(0, 0, 0, 0.82)";
                this.view.__overlay__.style.shadowOffsetX = -6;
                this.view.__overlay__.style.shadowOffsetY = 0;
            }

            open() {
                var side = this;

                var dim = side.getDimensions(),
                    sidewidth = dim.width;

                side.left = -sidewidth;
                side.opacity = __opacity_lo__;
                setAnimation(
                    c => {
                        c.left = 0;
                        c.opacity = __opacity_hi__;
                    },
                    __next_duration__,
                    Easing.Exponential.Out,
                    side
                );
                this.view.__overlay__.open(__next_duration__);


                this.view.left = 0;
                setAnimation(
                    c => {
                        c.left = sidewidth
                    },
                    __next_duration__,
                    Easing.Exponential.Out,
                    this.view
                );
            }

            close() {
                var side = this;

                var dim = side.getDimensions(),
                    sidewidth = dim.width;

                side.left = 0;
                side.opacity = __opacity_hi__;
                setAnimation(
                    c => {
                        c.left = -sidewidth;
                        c.opacity = __opacity_lo__;
                    },
                    __back_duration__,
                    Easing.Exponential.Out,
                    side
                );
                this.view.__overlay__.close(__back_duration__);


                this.view.left = sidewidth;
                setAnimation(
                    c => {
                        c.left = 0
                    },
                    __back_duration__,
                    Easing.Exponential.Out,
                    this.view
                );
            }
        }
    </script>
</component>
