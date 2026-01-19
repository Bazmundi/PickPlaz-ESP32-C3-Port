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
SINE_SPEED = 55  # src/pickplaz_app.c: sine_speed
LED_SAMPLE_INDEX = APP_SINE_LEN // 4  # 90-degree sample in a 256-entry table
TIME_SCALE = 1.0  # increase to slow the animation for viewing

LED_OFFSETS = (
    ("LED0", 0, BLUE),
    ("LED1", SINE_SPEED, GREEN),
    ("LED2", SINE_SPEED * 2, ORANGE),
    ("LED3", SINE_SPEED * 3, RED),
)


def sample_sintab(index: float) -> float:
    base = math.floor(index)
    frac = index - base
    i0 = int(base) % APP_SINE_LEN
    i1 = (i0 + 1) % APP_SINE_LEN
    return (1.0 - frac) * SINTAB[i0] + frac * SINTAB[i1]


def shifted_values(offset: int, phase: float) -> list[float]:
    return [sample_sintab(i + offset - phase) for i in range(APP_SINE_LEN)]


def shifted_values_reverse(offset: int, phase: float) -> list[float]:
    return [sample_sintab(i + offset + phase) for i in range(APP_SINE_LEN)]


def led_intensity(offset: int, phase: float) -> float:
    value = sample_sintab(LED_SAMPLE_INDEX + offset - phase)
    return value / SINTAB_MAX


def led_intensity_reverse(offset: int, phase: float) -> float:
    value = sample_sintab(LED_SAMPLE_INDEX + offset + phase)
    return value / SINTAB_MAX


class LedPhaseScrollScene(Scene):
    def construct(self) -> None:
        phase = ValueTracker(0.0)

        title = Text("LED Phase Offsets (left-to-right scroll)", font_size=28)
        title.to_edge(UP, buff=0.25)

        led_row = VGroup()
        for name, offset, color in LED_OFFSETS:
            led = Circle(radius=0.22, color=color, stroke_width=2)
            led.set_fill(color, opacity=0.05)

            def _update_led(mob, led_offset=offset, led_color=color):
                intensity = led_intensity(led_offset, phase.get_value())
                mob.set_fill(led_color, opacity=0.05 + 0.95 * intensity)

            led.add_updater(_update_led)
            label = Text(name, font_size=18)
            label.next_to(led, DOWN, buff=0.08)
            led_row.add(VGroup(led, label))

        led_row.arrange(RIGHT, buff=0.7)

        rows = VGroup()
        x_values = list(range(APP_SINE_LEN))
        for name, offset, color in LED_OFFSETS:
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
            label = Text(f"{name} (t + {offset})", font_size=20)
            label.next_to(axes, UP, buff=0.1)
            rows.add(VGroup(axes, graph, label))

        rows.arrange(DOWN, buff=0.25)

        content = VGroup(led_row, rows)
        content.arrange(DOWN, buff=0.35)
        max_content_height = config.frame_height - title.height - 0.8
        content.scale_to_fit_height(max_content_height)
        content.next_to(title, DOWN, buff=0.25)

        self.add(title)
        self.add(led_row)
        for row in rows:
            self.add(row)

        duration = (APP_SINE_LEN / APP_TICK_HZ) * TIME_SCALE
        self.play(phase.animate.set_value(APP_SINE_LEN), run_time=duration, rate_func=linear)
        self.wait(0.5)


class LedPhaseScrollReverseScene(Scene):
    def construct(self) -> None:
        phase = ValueTracker(0.0)

        title = Text("LED Phase Offsets (right-to-left scroll)", font_size=28)
        title.to_edge(UP, buff=0.25)

        led_row = VGroup()
        for name, offset, color in LED_OFFSETS:
            led = Circle(radius=0.22, color=color, stroke_width=2)
            led.set_fill(color, opacity=0.05)

            def _update_led(mob, led_offset=offset, led_color=color):
                intensity = led_intensity_reverse(led_offset, phase.get_value())
                mob.set_fill(led_color, opacity=0.05 + 0.95 * intensity)

            led.add_updater(_update_led)
            label = Text(name, font_size=18)
            label.next_to(led, DOWN, buff=0.08)
            led_row.add(VGroup(led, label))

        led_row.arrange(RIGHT, buff=0.7)

        rows = VGroup()
        x_values = list(range(APP_SINE_LEN))
        for name, offset, color in LED_OFFSETS:
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
                    shifted_values_reverse(offset, phase.get_value()),
                    line_color=color,
                    add_vertex_dots=False,
                )
            )
            label = Text(f"{name} (t + {offset})", font_size=20)
            label.next_to(axes, UP, buff=0.1)
            rows.add(VGroup(axes, graph, label))

        rows.arrange(DOWN, buff=0.25)

        content = VGroup(led_row, rows)
        content.arrange(DOWN, buff=0.35)
        max_content_height = config.frame_height - title.height - 0.8
        content.scale_to_fit_height(max_content_height)
        content.next_to(title, DOWN, buff=0.25)

        self.add(title)
        self.add(led_row)
        for row in rows:
            self.add(row)

        duration = (APP_SINE_LEN / APP_TICK_HZ) * TIME_SCALE
        self.play(phase.animate.set_value(APP_SINE_LEN), run_time=duration, rate_func=linear)
        self.wait(0.5)
