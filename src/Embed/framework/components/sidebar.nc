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

        const __opacity_lo__ = 0.8;
        const __opacity_hi__ = 1.0;
        const __next_duration__ = 450;
        const __back_duration__ = 420;

        const Elements = require('Elements');

        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                this.sidewidth = attr.width || 320;

                this.style.position = "relative";
                this.style.left = 0;
                this.style.width = 0;
                this.style.height = "100%";
                this.style.minWidth = 0.5*this.sidewidth;
            }

            attach(view) {
                this.view = view;
                this.view.sidebar = this;
                this.createViewOverlay();
                this.setEvents();
            }

            createViewOverlay() {
                if (this.view && this.view.__overlay__) return false;

                var overlay = new Elements.overlay({
                    opacity : 0.12
                });

                overlay.on("mousedown", () => {
                    this.close();
                });

                this.view.add(overlay);

                this.view.__overlay__ = overlay;

                this.view.__overlay__.style.coating = 40;
                this.view.__overlay__.style.shadowBlur = 20;
                this.view.__overlay__.style.shadowColor = "rgba(0, 0, 0, 0.82)";
                this.view.__overlay__.style.shadowOffsetX = -12;
                this.view.__overlay__.style.shadowOffsetY = 0;
            }

            setEvents() {
                document.canvas.on("mousedown", (e) => {
                    this._start = true;
                    this._moving = false;

                    this._event = {
                        x : e.x,
                        y : e.y
                    };
                });

                document.canvas.on("mousemove", (e) => {
                    if (!this._start) return false;

                    let dx = e.x - this._event.x;

                    if (Math.abs(dx) > 5) {
                        this._moving = true;
                        this.slide(dx);
                    }
                });

                document.canvas.on("mouseup", (e) => {
                    if (!this._start) return false;

                    this._start = false;

                    if (this._moving) {
                        let dx = e.x - this._event.x;
                        if (dx < 0.90*this.sidewidth) {
                            this.close();
                        } else {
                            this.open();
                        }
                    }

                    this._moving = false;
                });
            }

            slide(x) {
                var dim = side.getDimensions(),
                    sidewidth = dim.width,
                    currLeft = dim.left;

                if (this.width<=this.sidewidth) {
                    var opacity = (x/this.sidewidth) * (1-__opacity_lo__);

                    this.opacity = __opacity_lo__+opacity;
                    this.width = x;
                    this.view.left = this.width;

                    this.view.__overlay__.open(__next_duration__);
                }
            }

            open() {
                var side = this;

                setAnimation(
                    (side, view) => {
                        side.width = this.sidewidth;
                        side.opacity = __opacity_hi__;
                        view.left = this.sidewidth;
                    },
                    __next_duration__,
                    Easing.Exponential.Out,
                    side,
                    this.view
                );
                this.view.__overlay__.open(__next_duration__);
            }

            close() {
                var side = this;

                setAnimation(
                    (c, v) => {
                        c.width = 0;
                        c.opacity = __opacity_lo__;
                        v.left = 0;
                    },
                    __back_duration__,
                    Easing.Exponential.Out,
                    side,
                    this.view
                );
                this.view.__overlay__.close(__back_duration__);

            }
        }
    </script>
</component>
