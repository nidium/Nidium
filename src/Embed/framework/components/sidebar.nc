<component name="sidebar">
    <nss>
        bar: {
            backgroundColor: "#282728",
            position: "relative",
            height: "100%",

            display: "flex",
            flexDirection: "column",
            flexGrow: 1,
            justifyContent: "flex-start",
            alignItems: "stretch",
        }
    </nss>
    <layout class="bar">
        <slot></slot>
    </layout>
    <script>

        const __opacity_lo__ = 0.9;
        const __opacity_hi__ = 1.0;
        const __next_duration__ = 450;
        const __back_duration__ = 420;

        const Elements = require('Elements');

        module.exports = class extends Component {
            constructor(attr={}) {
                super(attr);

                this.sidewidth = attr.width || (window.innerWidth-150);

                this.style.position = "relative";
                this.style.left = 0;
                this.style.width = this.sidewidth;
                this.style.height = "100%";
                this.style.minWidth = 0.5*this.sidewidth;
                this.scrollableY = true;

                this._opened = false;
                
                this.hide();
                this.style.display = "none";
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

                this.view.__overlay__ = overlay;
                this.view.__overlay__.style.coating = 40;
                this.view.__overlay__.style.shadowBlur = 20;
                this.view.__overlay__.style.shadowColor = "rgba(0, 0, 0, 0.82)";
                this.view.__overlay__.style.shadowOffsetX = -12;
                this.view.__overlay__.style.shadowOffsetY = 0;
                this.view.add(overlay);

            }

            setEvents() {
                document.canvas.on("mousedown", (e) => {
                    if (this._opened) return;

                    this._start = true;
                    this._slided = false;
                    this._enabled = false;

                    this._dat = +new Date();

                    this._event = {
                        x : e.x,
                        y : e.y
                    };
                });

                document.canvas.on("mousemove", (e) => {
                    if (!this._start || this._opened) return false;

                    let dx = e.x - this._event.x;
                    let dy = e.y - this._event.y;

                    let ox = Math.abs(e.xrel);
                    let oy = Math.abs(e.yrel);


                    if (!this._slided && oy>1 && ox<=1) {
                        this._enabled = false;
                    }

                    if (Math.abs(dx)>50) {
                        this._enabled = true;
                    }

                    if (this._enabled) {
                        this._slided = true;
                        this.slide(dx);
                    }
                });

                document.canvas.on("mouseup", (e) => {
                    if (!this._start || !this._slided) {
                        this._start = false;
                        this._slided = false;
                        this._enabled = false
                        return false;
                    }

                    let dx = e.x - this._event.x;
                    let time = (+new Date()) - this._dat;

                    if (this._slided) {
                        if (time>160) {
                            if (dx < 0.70*this.sidewidth) {
                                this.close();
                            } else {
                                this.open();
                            }
                        } else {
                            if (dx>20) {
                                this.open();
                            } else {
                                this.close();
                            }
                        }
                    }

                    this._start = false;
                    this._slided = false;
                    this._enabled = false
                });
            }

            slide(x) {
                if (x<0) return false;

                if (x<=this.sidewidth) {
                    this.show();
                    this.style.display = "flex";

                    var opacity = (x/this.sidewidth) * (1-__opacity_lo__);

                    this.opacity = __opacity_lo__+opacity;
                    //this.width = x;
                    this.view.left = x;

                    this.view.__overlay__.open(__next_duration__);
                }
               
            }

            open() {
                var side = this;

                this.show();
                this.style.display = "flex";

                this._opened = true;

                this.anim = setAnimation(
                    (side, view) => {
                        //side.width = this.sidewidth;
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

            close(callback=function(){}) {
                var side = this;

                this._opened = false;

                this.show();
                this.anim = setAnimation(
                    (side, view) => {
                        side.opacity = __opacity_lo__;
                        view.left = 0;
                    },
                    __back_duration__,
                    Easing.Exponential.Out,
                    side,
                    this.view
                );

                this.anim.onFinish = () => {
                    this.hide();
                    this.style.display = "none";
                    callback.call(this);
                };

                this.view.__overlay__.close(__back_duration__);

            }
        }
    </script>
</component>
