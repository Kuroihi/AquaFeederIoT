<script>
document.addEventListener("DOMContentLoaded", function () {
    const scheduleForm = document.getElementById("schedule-form");
    const scheduleInput = document.getElementById("schedule-input");
    const scheduleList = document.getElementById("schedule-list");

    let schedules = [];

    // Ambil jadwal awal dari elemen tersembunyi
    try {
        const rawData = document.getElementById("initial-schedules").textContent;
        schedules = JSON.parse(rawData);
    } catch (e) {
        console.error("Gagal memuat jadwal awal");
    }

    renderSchedules();

    // Tambah jadwal baru
    scheduleForm.addEventListener("submit", function (e) {
        e.preventDefault();
        const time = scheduleInput.value;
        if (!time) return;

        // Tambahkan ke array dan update ke server
        schedules.push({ time, active: true });

        fetch("/set_schedule", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ schedules }),
        })
        .then(res => res.json())
        .then(() => {
            renderSchedules();
            scheduleInput.value = "";
        });
    });

    // Render daftar jadwal
    function renderSchedules() {
        scheduleList.innerHTML = "";
        schedules.forEach((s, index) => {
            const li = document.createElement("li");
            li.textContent = s.time;
            scheduleList.appendChild(li);
        });
    }
});
</script>
