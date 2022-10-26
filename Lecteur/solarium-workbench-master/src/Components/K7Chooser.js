import React from "react";
import {
    Select,
    MenuItem
} from '@material-ui/core';

export default function K7Chooser(props) {
    const [cassetteName, setCassetteName] = React.useState(props.defaultK7 || "K701BT_SE");

    const handleSelectionChanged = (event) => {
        setCassetteName(event.target.value);
        if (props.onSelectionChanged) {
            props.onSelectionChanged(event.target.value);
        }
    }

    return (
        <Select
            value={cassetteName}
            onChange={handleSelectionChanged}
            label="Cassette name"
        >
            <MenuItem value="K701BT_SE">K701BT_SE</MenuItem>
            <MenuItem value="K701WT_SE">K701WT_SE</MenuItem>
            <MenuItem value="K701BT_BL">K701BT_BL</MenuItem>
            <MenuItem value="K701WT_BL">K701WT_BL</MenuItem>
            <MenuItem value="K701BT_NL">K701BT_NL</MenuItem>
            <MenuItem value="K701WT_NL">K701WT_NL</MenuItem>
        </Select>
    );
}