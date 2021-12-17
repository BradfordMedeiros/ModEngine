<html>
  <body>
    <div class="title">ModEngine Perf</div>
    <div class="content">
      <h1>Graphs</h1>
      <div class="graph_container">
        <?php
          $index = 0;
          foreach (glob("*.png") as $filename) {
             echo '<div class="content_item">';
             echo "<div onclick=\"toggleClass('#membenchmark_image_" . $index . "', 'hidden')\" class=\"toggle\">$filename</div>";
             echo "<img id=\"" . "membenchmark_image_" . $index . "\"src=\"$filename\" />";
             echo "</div>";
             $index = $index + 1;
          }
        ?>
        </div>
      </div>
    </div>

  </body>

  <style>
    body {
      background: rgb(30, 30, 30);
      margin: 0;
    }
    .title {
      background: black;
      color: whitesmoke;
      padding: 1em;
      font-size: 1.5em;
    }
    h1 {
      color: white;
      padding-left: 1em;
    }
    .content {
      display: flex;
      flex-direction: column;
    }
    .graph_container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
    }
    .toggle {
      padding: 1em;
    }
    .content_item {
      display: flex;
      flex-direction: column;
      box-shadow: 0px 1px 10px black;
      color: steelblue;
      background: black;
      margin: 24px;
      width: 40em;
    }
    .toggle { cursor: pointer; }
    .hidden {
      display: none;
    }
  </style>

  <script> 
    const toggleClass = (selectorName, value) => {
      console.log("selector name: ", selectorName)
      const element = document.querySelector(selectorName);
      element.classList.contains("hidden") ? element.classList.remove("hidden") : element.classList.add("hidden");
    }

  </script>
</html>
