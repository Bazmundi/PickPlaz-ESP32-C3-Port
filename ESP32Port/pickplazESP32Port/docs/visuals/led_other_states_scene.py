import math

from manim import (
    Axes,
    BLUE,
    Circle,
    DOWN,
    GREEN,
    ORANGE,
    RED,
    RIGHT,
    Scene,
    Text,
    UP,
    VGroup,
    ValueTracker,
    always_redraw,
    config,
    linear,
)

from sine_wave_scene import SINTAB

APP_SINE_LEN = len(SINTAB)
SINTAB_MAX = max(SINTAB) if SINTAB else 1
APP_TICK_HZ = 1000  # app_tick() runs at 1 kHz
LED_SAMPLE_INDEX = APP_SINE_LEN // 4  # 90-degree sample in a 256-entry table
TIME_SCALE = 1.0
LEVEL_MAX = "max"

LED_COLORS = {
    "LED0": BLUE,
    "LED1": GREEN,
    "LED2": ORANGE,
    "LED3": RED,
}


def sample_sintab(index: float) -> float:
    base = math.floor(index)
    frac = index - base
    i0 = int(base) % APP_SINE_LEN
    i1 = (i0 + 1) % APP_SINE_LEN
    return (1.0 - frac) * SINTAB[i0] + frac * SINTAB[i1]


def shifted_values(offset: int | None | str, phase: float) -> list[float]:
    if offset is None:
        return [0.0] * APP_SINE_LEN
    if offset == LEVEL_MAX:
        return [float(SINTAB_MAX)] * APP_SINE_LEN
    return [sample_sintab(i + offset - phase) for i in range(APP_SINE_LEN)]


def led_intensity(offset: int | None | str, phase: float) -> float:
    if offset is None:
        return 0.0
    if offset == LEVEL_MAX:
        return 1.0
    value = sample_sintab(LED_SAMPLE_INDEX + offset - phase)
    return value / SINTAB_MAX


def build_led_row(offsets: tuple[tuple[str, int | None | str], ...], phase: ValueTracker) -> VGroup:
    led_row = VGroup()
    for name, offset in offsets:
        color = LED_COLORS[name]
        led = Circle(radius=0.22, color=color, stroke_width=2)
        led.set_fill(color, opacity=0.05)

        def _update_led(mob, led_offset=offset, led_color=color):
            intensity = led_intensity(led_offset, phase.get_value())
            mob.set_fill(led_color, opacity=0.05 + 0.95 * intensity)

        if offset is None:
            pass
        elif offset == LEVEL_MAX:
            led.set_fill(color, opacity=1.0)
        else:
            led.add_updater(_update_led)
        label = Text(name, font_size=18)
        label.next_to(led, DOWN, buff=0.08)
        led_row.add(VGroup(led, label))

    led_row.arrange(RIGHT, buff=0.7)
    return led_row


def build_graph_rows(
    offsets: tuple[tuple[str, int | None | str], ...],
    phase: ValueTracker,
) -> VGroup:
    rows = VGroup()
    x_values = list(range(APP_SINE_LEN))
    for name, offset in offsets:
        color = LED_COLORS[name]
        axes = Axes(
            x_range=[0, APP_SINE_LEN - 1, 64],
            y_range=[0, 260, 64],
            x_length=8.6,
            y_length=1.0,
            tips=False,
        )
        graph = always_redraw(
            lambda axes=axes, offset=offset, color=color: axes.plot_line_graph(
                x_values,
                shifted_values(offset, phase.get_value()),
                line_color=color,
                add_vertex_dots=False,
            )
        )
        if offset is None:
            label_text = f"{name} (off)"
        elif offset == LEVEL_MAX:
            label_text = f"{name} (max)"
        else:
            label_text = f"{name} (t + {offset})"
        label = Text(label_text, font_size=20)
        label.next_to(axes, UP, buff=0.1)
        rows.add(VGroup(axes, graph, label))

    rows.arrange(DOWN, buff=0.25)
    return rows


def layout_scene(title_text: str, led_row: VGroup, rows: VGroup) -> VGroup:
    title = Text(title_text, font_size=28)
    title.to_edge(UP, buff=0.25)

    content = VGroup(led_row, rows)
    content.arrange(DOWN, buff=0.35)
    max_content_height = config.frame_height - title.height - 0.8
    content.scale_to_fit_height(max_content_height)
    content.next_to(title, DOWN, buff=0.25)

    return VGroup(title, content)


class IdleIndexedScene(Scene):
    def construct(self) -> None:
        offsets = (
            ("LED0", None),
            ("LED1", None),
            ("LED2", None),
            ("LED3", LEVEL_MAX),
        )
        phase = ValueTracker(0.0)
        led_row = build_led_row(offsets, phase)
        rows = build_graph_rows(offsets, phase)
        title, content = layout_scene("Idle (indexed): LED3 steady on", led_row, rows)

        self.add(title)
        self.add(content)
        self.wait(2)


class IdleUnindexedScene(Scene):
    def construct(self) -> None:
        phase = ValueTracker(0.0)
        offsets = (("LED0", None), ("LED1", 0), ("LED2", 128), ("LED3", None))
        led_row = build_led_row(offsets, phase)
        rows = build_graph_rows(offsets, phase)
        title, content = layout_scene("Idle (unindexed): LED1/LED2 180° apart", led_row, rows)

        self.add(title)
        self.add(led_row)
        for row in rows:
            self.add(row)

        duration = (APP_SINE_LEN / APP_TICK_HZ) * TIME_SCALE
        self.play(phase.animate.set_value(APP_SINE_LEN), run_time=duration, rate_func=linear)
        self.wait(0.5)


class DefaultSineScene(Scene):
    def construct(self) -> None:
        phase = ValueTracker(0.0)
        offsets = (("LED0", 0), ("LED1", 128), ("LED2", 256), ("LED3", 384))
        led_row = build_led_row(offsets, phase)
        rows = build_graph_rows(offsets, phase)
        title, content = layout_scene("Default sine: 180° pair offsets", led_row, rows)

        self.add(title)
        self.add(led_row)
        for row in rows:
            self.add(row)

        duration = (APP_SINE_LEN / APP_TICK_HZ) * TIME_SCALE
        self.play(phase.animate.set_value(APP_SINE_LEN), run_time=duration, rate_func=linear)
        self.wait(0.5)
